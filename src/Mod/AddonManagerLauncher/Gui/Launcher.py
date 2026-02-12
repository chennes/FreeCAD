# SPDX-License-Identifier: LGPL-2.1-or-later
# SPDX-FileCopyrightText: 2026 The FreeCAD project association AISBL
# SPDX-FileNotice: Part of the FreeCAD project.
import os
import pathlib
import runpy

from PySide import QtCore, QtWidgets
import FreeCAD
import FreeCADGui
from AddonManagerLauncher.App.Downloader import Downloader

translate = FreeCAD.Qt.translate


def QT_TRANSLATE_NOOP(context: str, string: str):
    pass


class CommandAddonManagerLauncher(QtCore.QObject):

    def __init__(self):
        super().__init__()
        self.params = FreeCAD.ParamGet(
            "User parameter:BaseApp/Preferences/Mod/AddonManagerLauncher"
        )
        self.downloader = Downloader()
        self.download_thread = None
        self.installing_dialog = None

        if self.needs_installation():
            self.show_dialog()
        else:
            self.run_addon_manager()

    def GetResources(self) -> dict[str, str]:
        """FreeCAD-required function: get the core resource information for this Mod."""
        return {
            "Pixmap": "AddonManagerLauncher",
            "MenuText": QT_TRANSLATE_NOOP("Std_AddonMgrLauncher", "&Addon Manager"),
            "ToolTip": QT_TRANSLATE_NOOP(
                "Std_AddonMgrLauncher",
                "Manages external workbenches, macros, and preference packs",
            ),
            "Group": "Tools",
        }

    def Activated(self) -> None:
        """FreeCAD-required function: called when the command is activated."""
        if self.needs_installation():
            self.show_dialog()
        else:
            self.run_addon_manager()

    def needs_installation(self) -> bool:
        return self.params.GetBool("am_downloaded", False)

    def show_dialog(self):
        warning_dialog = FreeCADGui.PySideUic.loadUi(
            os.path.join(
                os.path.dirname(__file__), "Resources", "dialogs", "installation_warning.ui"
            )
        )

        warning_dialog.setDefaultButton(QtWidgets.QDialogButtonBox.OK)
        result = warning_dialog.exec_()
        if result == QtWidgets.QDialogButtonBox.OK:
            self.install_addon_manager()

    def install_addon_manager(self):
        self.installing_dialog = FreeCADGui.PySideUic.loadUi(
            os.path.join(os.path.dirname(__file__), "Resources", "dialogs", "installing.ui")
        )

        self.downloader.download_complete.connect(self._download_complete)
        self.downloader.download_failed.connect(self._download_failed)
        self.downloader.download_progress.connect(self._download_progress)

        QtCore.QThreadPool.globalInstance().start(self.downloader)

        result = self.installing_dialog.exec_()
        if result == QtWidgets.QMessageBox.Cancel:
            self.downloader.cancel()

    @QtCore.Slot()
    def _cancel_download(self):
        self.downloader.stop()
        self.installing_dialog.rejected.disconnect()
        self.installing_dialog.reject()

    @QtCore.Slot()
    def _download_complete(self, path: str):
        self.installing_dialog.close()
        init = pathlib.Path(path) / "Init.py"
        init_gui = pathlib.Path(path) / "InitGui.py"
        if init.exists():
            runpy.run_path(str(init))
        if init_gui.exists():
            runpy.run_path(str(init_gui))
        self.run_addon_manager()

    @QtCore.Slot()
    def _download_failed(self, error: str):
        self.installing_dialog.close()
        QtWidgets.QMessageBox.warning(
            None, "AddonManagerLauncher", error, QtWidgets.QMessageBox.Close
        )

    @QtCore.Slot()
    def _download_progress(self, current: int, total: int):
        self.installing_dialog.progress_bar.setMaximum(total)
        self.installing_dialog.progress_bar.setValue(current)

    def run_addon_manager(self):
        # Whichever addon manager is installed should have declared this command:
        FreeCADGui.runCommand("Std_AddonMgr")
