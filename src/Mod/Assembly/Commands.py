# -*- coding: utf-8 -*-
# ***************************************************************************
# *   Copyright (c) 2023 Ondsel <development@ondsel.com>                    *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU Lesser General Public License (LGPL)    *
# *   as published by the Free Software Foundation; either version 2 of     *
# *   the License, or (at your option) any later version.                   *
# *   for detail see the LICENCE text file.                                 *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this program; if not, write to the Free Software   *
# *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
# *   USA                                                                   *
# *                                                                         *
# ***************************************************************************

import FreeCAD as App

from PySide.QtCore import QT_TRANSLATE_NOOP

if App.GuiUp:
    import FreeCADGui as Gui
    from PySide import QtCore
    from PySide import QtGui

# translate = App.Qt.translate

__title__ = "Assembly Commands"
__author__ = "Ondsel"
__url__ = "https://www.freecad.org"


class CommandCreateAssembly:

    def __init__(self):
        pass

    def GetResources(self):
        return {
            "Pixmap": "Geoassembly",
            "MenuText": QT_TRANSLATE_NOOP("Assembly_CreateAssembly", "Create Assembly"),
            "Accel": "A",
            "ToolTip": QT_TRANSLATE_NOOP(
                "Assembly_CreateAssembly", "Create an assembly object in the current document."
            ),
            "CmdType": "ForEdit",
        }

    def IsActive(self):
        return App.ActiveDocument is not None

    def Activated(self):
        assembly = App.ActiveDocument.addObject('App::Part','Assembly')
        assembly.Type = 'Assembly'


class CommandInsertLink:

    def __init__(self):
        pass

    def GetResources(self):
        tooltip  = "<p>Insert a Link into the assembly. "
        tooltip += "This will create a dynamic link to a part/body/primitive/assembly, "
        tooltip += "which can be in this document or in another document "
        tooltip += "that is open in the current session</p>"
        tooltip += "<p><b>Note</b>: the part must be open in the current session</p>"
        tooltip += "<p>This command also enables to repair broken/missing links. "
        tooltip += "Select the broken link, launch this command, and select a new target part in the list</p>"

        return {
            "Pixmap": "Assembly_InsertLink.svg",
            "MenuText": QT_TRANSLATE_NOOP("Assembly_InsertLink", "Insert Link"),
            "Accel": "I",
            "ToolTip": QT_TRANSLATE_NOOP("Assembly_InsertLink", tooltip),
            "CmdType": "ForEdit",
        }

    def IsActive(self):
        doc = Gui.ActiveDocument

        if (doc is None or doc.ActiveView is None):
            return False

        active_part, parent, sub = doc.ActiveView.getActiveObject('part', False)

        if (active_part is not None and active_part.Type == "Assembly"):
            return True

        return False

    def Activated(self):
        print("Insert Link command clicked. Not implemented yet.")


if App.GuiUp:
    Gui.addCommand("Assembly_CreateAssembly", CommandCreateAssembly())
    Gui.addCommand("Assembly_InsertLink", CommandInsertLink())


