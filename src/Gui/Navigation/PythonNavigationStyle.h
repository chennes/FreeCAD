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

#include "Navigation/NavigationStyle.h"

#include <map>
#include <vector>
#include <string>

#include <CXX/Objects.hxx>

namespace Gui
{
class PythonNavigationStyleRegistry
{
public:
    void addStyle(const std::string& name, const Py::Object& pythonClass);
    void removeStyle(const std::string& name);
    std::vector<std::string> getStyles() const;
    Py::Object getStyle(const std::string& name);

private:
    std::map<std::string, Py::Object> _styles;
};

class PythonNavigationStyle: public UserNavigationStyle
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    PythonNavigationStyle() = default;

    const char* mouseButtons(ViewerMode mode) override;

    SbBool processSoEvent(const SoEvent* ev) override;

    static PythonNavigationStyleRegistry& registry();

private:
    static PythonNavigationStyleRegistry _registry;
    PyObject* proxy {nullptr};
};
}  // namespace Gui
