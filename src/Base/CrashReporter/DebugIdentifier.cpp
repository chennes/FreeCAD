// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2026 The FreeCAD project association AISBL
// SPDX-FileNotice: Part of the FreeCAD project.

/******************************************************************************
 *                                                                            *
 *   FreeCAD is free software: you can redistribute it and/or modify          *
 *   it under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1            *
 *   of the License, or (at your option) any later version.                   *
 *                                                                            *
 *   FreeCAD is distributed in the hope that it will be useful,               *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty              *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *   See the GNU Lesser General Public License for more details.              *
 *                                                                            *
 *   You should have received a copy of the GNU Lesser General Public         *
 *   License along with FreeCAD. If not, see https://www.gnu.org/licenses     *
 *                                                                            *
 ******************************************************************************/
#include "DebugIdentifier.h"

#include <FCConfig.h>
#include <uuid/uuid.h>

#ifdef FC_OS_MACOSX
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach/machine.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>
#include <libkern/OSByteOrder.h>
#endif

using namespace Base::CrashReporter;

namespace
{
#if defined (FC_OS_MACOSX)

std::uintmax_t getStreamLength(std::ifstream &stream)
{
    auto oldPosition = stream.tellg();
    stream.seekg(0, std::ios::end);
    if (!stream.good()) {
        return 0;
    }
    const std::uintmax_t fileSize = stream.tellg();
    stream.seekg(oldPosition);
    return fileSize;
}

/**
 * Get the position of the Mach-O record we want to read to get the debug identifier
 *
 * @param moduleStream An open stream into the module, at position 0.
 * @param architecture The crash file architecture.
 * @return The fat_arch struct for the record we need to read, or nullopt if it cannot be found
 */
std::optional<fat_arch> findFatOffset(
    std::ifstream &moduleStream, std::int32_t targetArch)
{
    // fat, big-endian fields: select slice by looping through available architectures
    // and match the one this crash record recorded from
    std::array<uint32_t, 2> buffer;
    constexpr std::uint32_t nArchMax{16}; // Sanity check - normally this is 1, 2, or 3
    moduleStream.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
    if (!moduleStream.good()) {
        return std::nullopt;
    }
    std::uint32_t nFatArch = OSReadBigInt32(buffer.data(), 4);
    const std::uintmax_t fileSize = getStreamLength(moduleStream);
    if (nFatArch > nArchMax || nFatArch > (fileSize-8) / sizeof(fat_arch)) {
        // What is this thing? Corrupt or malicious...
        return std::nullopt;
    }

    // Buffered read of all the arch structures so we can convert them to native endian
    std::vector<char> archBuffer(nFatArch*sizeof(fat_arch));
    moduleStream.read(archBuffer.data(), archBuffer.size());
    if (!moduleStream.good()) {
        return std::nullopt;
    }

    for (std::uint32_t i = 0; i < nFatArch; ++i) {
        fat_arch arch {};
        char* baseAddress = archBuffer.data() + i*sizeof(fat_arch);  // NOLINT
        arch.cputype = OSReadBigInt32(baseAddress, 0);
        arch.cpusubtype = OSReadBigInt32(baseAddress, 4);
        arch.offset = OSReadBigInt32(baseAddress, 8);
        arch.size = OSReadBigInt32(baseAddress, 12);
        arch.align = OSReadBigInt32(baseAddress, 16);
        if (arch.cputype == targetArch) {
            if (arch.offset > fileSize || arch.size > fileSize - arch.offset) { // Don't overflow!
                return std::nullopt;
            }
            return arch;
        }
    }
    return std::nullopt;
}

/**
 * Read the Mach-O file in `moduleStream`, starting at `offset`, and try to extract the
 * Debug ID from it (as used by symbolication services such as the one at Sentry to identify the
 * right symbol lookup).
 *
 * @param offset Read the record beginning at this offset into the file
 * @param limit Read at most this number of bytes
 * @param moduleStream The stream to read from (positioned anywhere in the stream, we `seekg()`)
 * @param targetArch The architecture we found in the crash file: **must** match the arch record
 * that this `offset` points at.
 * @return The debug identifier, if it can be found, or nullopt if it cannot (or there is an error)
 */
std::optional<DebugIdentifier> readThinMachOAt(
    std::uint32_t offset,
    std::uintmax_t limit,
    std::ifstream &moduleStream,
    std::int32_t targetArch)
{
    moduleStream.seekg(offset);
    if (!moduleStream.good()) {
        return std::nullopt;
    }

    mach_header_64 header {};
    moduleStream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!moduleStream.good()) {
        return std::nullopt;
    }

    if (header.magic != MH_MAGIC_64) {
        return std::nullopt;
    }

    if (header.cputype != targetArch) {
        return std::nullopt;
    }

    if (limit < sizeof(mach_header_64)
        || header.sizeofcmds > limit - sizeof(mach_header_64)) { // Avoid underflow
        return std::nullopt;
    }

    std::streamoff position = static_cast<std::streamoff>(offset) + sizeof(mach_header_64);
    std::uint32_t consumed = 0;

    for (std::uint32_t i = 0; i < header.ncmds; ++i) {
        moduleStream.seekg(position);
        if (!moduleStream.good()) {
            return std::nullopt;
        }

        load_command loadCommand {};
        moduleStream.read(reinterpret_cast<char*>(&loadCommand), sizeof(loadCommand));
        if (!moduleStream.good()) {
            return std::nullopt;
        }

        if (loadCommand.cmdsize < sizeof(load_command)
            || loadCommand.cmdsize > header.sizeofcmds - consumed) {
            return std::nullopt;
        }

        if (loadCommand.cmd == LC_UUID) {
            if (loadCommand.cmdsize != sizeof(uuid_command)) {
                // Corrupt, or malicious
                return std::nullopt;
            }
            // Found it! The stream is already positioned at the uuid field, which directly
            // follows the command header.
            uuid_t uuid {};
            moduleStream.read(reinterpret_cast<char*>(&uuid), sizeof(uuid));
            if (!moduleStream.good()) {
                return std::nullopt;
            }
            uuid_string_t uuidString {};
            uuid_unparse_lower(uuid, uuidString);
            std::string debugID {uuidString};
            std::string codeID {debugID};
            std::erase(codeID, '-');  // By definition, on Mach-O same as UUID but without '-'
            return DebugIdentifier {.debugID = debugID, .codeID = codeID};
        }

        consumed += loadCommand.cmdsize;
        position += loadCommand.cmdsize;
    }

    // Not every Mach-O carries an LC_UUID, so this is a normal case (don't treat it as an error!)
    return std::nullopt;
}
#endif
}


std::optional<DebugIdentifier> Base::CrashReporter::debugIdentifier(
    const std::string& modulePath,
    [[maybe_unused]] Architecture architecture)
{
#if defined (FC_OS_MACOSX)
    // We cannot use the (undocumented) `_dyld_get_image_uuid` method that other projects seem to
    // use for this because we expect it to be quite common that the crash happened in a module
    // that is not yet loaded at the time this code runs. Instead, we will use the module path to
    // directly load and inspect the module for the debug identifier that Sentry expects. This gets
    // a little painful...

    // Any error that happens along the way results in an early-return of std::nullopt...

    if (!std::filesystem::is_regular_file(modulePath)) {
        return std::nullopt;
    }

    std::ifstream moduleStream (modulePath, std::ios::binary);
    if (!moduleStream) {
        return std::nullopt;
    }

    std::int32_t targetArch;
    switch (architecture) {
        case Architecture::x64:
            targetArch = CPU_TYPE_X86_64;
            break;
        case Architecture::aarch64:
            targetArch = CPU_TYPE_ARM64;
            break;
        default:
            return std::nullopt;
    }
    try {
        std::uint32_t magic;
        moduleStream.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        if (!moduleStream.good()) {
            return std::nullopt;
        }
        switch (magic) {
            case MH_MAGIC_64:
                // thin, native byte order: direct read, no offset
                return readThinMachOAt(
                    0, getStreamLength(moduleStream), moduleStream, targetArch);
            case FAT_CIGAM:
                moduleStream.seekg(0);
                if (auto archRecord = findFatOffset(moduleStream, targetArch);
                    archRecord.has_value()) {
                    return readThinMachOAt(
                        archRecord.value().offset,
                        archRecord.value().size,
                        moduleStream,
                        targetArch);
                }
                return std::nullopt;
            default:
                return std::nullopt;
        }
    }
    catch (...) {
        return std::nullopt;
    }

#elif defined (FC_OS_WIN32)
#elif defined (FC_OS_LINUX)
#elif defined (FC_OS_BSD)
#endif
    return std::nullopt;
}
