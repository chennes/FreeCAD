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

import FreeCAD

from PySide.QtCore import QT_TRANSLATE_NOOP

if FreeCAD.GuiUp:
    import FreeCADGui
    from PySide import QtCore
    from PySide import QtGui

# translate = FreeCAD.Qt.translate

__title__ = "Assembly Commands"
__author__ = "Ondsel"
__url__ = "https://www.freecad.org"


class _dummy:
    "the dummy command for initial implementation"

    def __init__(self):
        self.obj = None
        self.sub = []
        self.active = False

    def GetResources(self):
        return {
            "Pixmap": "Assembly_Dummy",
            "MenuText": QT_TRANSLATE_NOOP("Assembly_Dummy", "dummy"),
            "Accel": "A, D",
            "ToolTip": QT_TRANSLATE_NOOP(
                "Assembly_Dummy", "dummy"
            ),
            "CmdType": "ForEdit",
        }

    def IsActive(self):
        return True

    def Activated(self):
        print ('did the dummy command')


if FreeCAD.GuiUp:
    FreeCADGui.addCommand("Assembly_Dummy", _dummy())


