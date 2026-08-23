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

#include <CXX/Python3/Objects.hxx>

#include <fmt/format.h>

#include <string>

#include "CrashReporter/CrashFramePy.h"
#include "CrashReporter/CrashFramePy.cpp"

using namespace Base;

std::string CrashFramePy::representation() const
{
    const PointerType frame = getParsedFramePtr();
    const std::string& module = frame->modulePath;
    return fmt::format(
        "<CrashFrame 0x{:016X} in {}: {}>",
        frame->rawAddress,
        module.empty() ? "<unknown module>" : module.c_str(),
        frame->symbol.has_value() ? frame->symbol->c_str() : "<unresolved>"
    );
}

Py::Long CrashFramePy::getaddress() const
{
    return Py::Long(getParsedFramePtr()->rawAddress);
}

Py::String CrashFramePy::getmodule() const
{
    return Py::String(getParsedFramePtr()->modulePath);
}

Py::Boolean CrashFramePy::getis_inline() const
{
    return Py::Boolean(getParsedFramePtr()->isInline);
}

Py::Object CrashFramePy::getmodule_offset() const
{
    const auto& moduleOffset = getParsedFramePtr()->moduleOffset;
    if (!moduleOffset.has_value()) {
        return Py::None();
    }
    return static_cast<Py::Object>(Py::Long(moduleOffset.value()));
}

Py::Object CrashFramePy::getsymbol() const
{
    const auto& symbol = getParsedFramePtr()->symbol;
    if (!symbol.has_value()) {
        return Py::None();
    }
    return static_cast<Py::Object>(Py::String(symbol.value()));
}

Py::Object CrashFramePy::getfile() const
{
    const auto& file = getParsedFramePtr()->file;
    if (!file.has_value()) {
        return Py::None();
    }
    return static_cast<Py::Object>(Py::String(file.value()));
}

Py::Object CrashFramePy::getline() const
{
    const auto& line = getParsedFramePtr()->line;
    if (!line.has_value()) {
        return Py::None();
    }
    // We need the inner static_cast to ensure that Py::Long uses the right constructor
    return static_cast<Py::Object>(Py::Long(static_cast<unsigned long>(line.value())));
}

PyObject* CrashFramePy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int CrashFramePy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}
