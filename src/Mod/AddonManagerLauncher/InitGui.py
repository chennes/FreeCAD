# SPDX-License-Identifier: LGPL-2.1-or-later
import os

import FreeCADGui
from AddonManagerLauncher.Gui.Launcher import CommandAddonManagerLauncher

FreeCADGui.addCommand("Std_AddonMgrLauncher", CommandAddonManagerLauncher())

cwd = os.path.dirname(AddonManagerLauncher.__file__)
FreeCADGui.addLanguagePath(os.path.join(cwd, "Resources", "translations"))
FreeCADGui.addIconPath(os.path.join(cwd, "Resources", "icons"))
FreeCADGui.addPreferencePage(
    os.path.join(cwd, "Resources", "preferences", "addonmanagerlauncherprefs.ui"),
    "Addon Manager Launcher",
)
