# SPDX-License-Identifier: LGPL-2.1-or-later
# AddonManager gui init module
# (c) 2001 Juergen Riegel
# License LGPL

import AddonManager

FreeCADGui.addLanguagePath(":/translations")
FreeCADGui.addCommand("Std_AddonMgr", AddonManager.CommandAddonManager())

try:
    import FreeCAD

    FreeCAD.__unit_test__ += ["TestAddonManagerGui"]
except ImportError:
    pass
