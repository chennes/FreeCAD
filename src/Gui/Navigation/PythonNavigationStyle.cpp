// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2011 Werner Mayer <wmayer[at]users.sourceforge.net>
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

#include "PythonNavigationStyle.h"


#include <Inventor/nodes/SoCamera.h>
#include <QApplication>

#include "Navigation/NavigationStyle.h"
#include "View3DInventorViewer.h"


using namespace Gui;


void PythonNavigationStyleRegistry::addStyle(const std::string& name, const Py::Object& pythonClass)
{
    if (_styles.contains(name)) {
        std::string msg = fmt::format("A navigation style called '%s' was already registered", name);
        throw Base::NameError(msg);
    }
    _styles[name] = pythonClass;
}

void PythonNavigationStyleRegistry::removeStyle(const std::string& name)
{
    if (_styles.contains(name)) {
        _styles.erase(name);
    }
}

std::vector<std::string> PythonNavigationStyleRegistry::getStyles() const
{
    std::vector<std::string> styles;
    styles.reserve(_styles.size());
    for (const auto& style : _styles) {
        styles.push_back(style.first);
    }
    return styles;
}

Py::Object PythonNavigationStyleRegistry::getStyle(const std::string& name)
{
    if (_styles.contains(name)) {
        return _styles[name];
    }
    return Py::None();
}


/* TRANSLATOR Gui::PythonNavigationStyle */

TYPESYSTEM_SOURCE(Gui::PythonNavigationStyle, Gui::UserNavigationStyle)

PythonNavigationStyleRegistry PythonNavigationStyle::_registry;

const char* PythonNavigationStyle::mouseButtons(ViewerMode mode)
{
    return nullptr;
}

SbBool PythonNavigationStyle::processSoEvent(const SoEvent* const ev)
{
    return false;
}

PythonNavigationStyleRegistry& PythonNavigationStyle::registry()
{
    return _registry;
}
