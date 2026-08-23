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

#pragma once

#include <optional>
#include <string>
#include <FCGlobal.h>

#include "Reader.h"

namespace Base::CrashReporter
{

struct DebugIdentifier
{
    std::string debugID;
    std::string codeID;
};

/**
 * Get a debug_id for a loaded module.
 *
 * On macOS this is the Mach-O LC_UUID; on Linux the GNU build-id; on Windows the PDB GUID + age.
 * Remote symbolicators (e.g. Sentry) use it to match a frame's module against an uploaded debug
 * file.
 *
 * @param modulePath The complete path to the module
 * @param architecture The architecture to resolve for, e.g. where the crash happened (really only
 * relevant for macOS "fat" binaries).
 * @return The DebugIdentifier as required by, e.g. Sentry (or std::nullopt if not found)
 */

[[nodiscard]] std::optional<DebugIdentifier> BaseExport debugIdentifier(
    const std::string& modulePath, Architecture architecture);

}  // namespace Base::CrashReporter
