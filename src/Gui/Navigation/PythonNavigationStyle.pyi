# SPDX-License-Identifier: LGPL-2.1-or-later

from __future__ import annotations

from Base.Metadata import export
from Gui.NavigationStyle import NavigationStyle

@export(
    Include="Gui/Navigation/PythonNavigationStyle.h",
    FatherInclude="Gui/Navigation/NavigationStylePy.h",
    Constructor=True,
)
class PythonNavigationStyle(NavigationStyle):
    pass
