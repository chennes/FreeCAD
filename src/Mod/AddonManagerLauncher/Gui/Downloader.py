from PySide import QtCore, QtNetwork

import pathlib
import tempfile
import zipfile


class Downloader(QtCore.QObject):
    download_complete = QtCore.Signal(str)
    download_failed = QtCore.Signal(str)
    download_progress = QtCore.Signal(int, int)
    download_cancelled = QtCore.Signal()

    def __init__(self, parent=None):
        super().__init__(parent)

        self.params = FreeCAD.ParamGet(
            "User parameter:BaseApp/Preferences/Mod/AddonManagerLauncher"
        )

        self._nam = QtNetwork.QNetworkAccessManager(self)
        self._reply = None
        self._tmp_file = None
        self._tmp_path = None
        self._am_url = ""

    def start(self):
        self._am_url = self.params.GetString("am_url") or ""
        if not self._am_url.strip():
            self.download_failed.emit("am_url parameter is empty, nothing to install")
            return

        url = QtCore.QUrl(self._am_url)
        if not url.isValid() or url.isRelative():
            self.download_failed.emit(f"Invalid URL: {self._am_url}")
            return

        if url.scheme() not in ("http", "https"):
            self.download_failed.emit(f"Unsupported URL scheme '{url.scheme()}': {self._am_url}")
            return

        req = QtNetwork.QNetworkRequest(url)
        req.setAttribute(QtNetwork.QNetworkRequest.FollowRedirectsAttribute, True)

        self._reply = self._nam.get(req)

        fd, path = tempfile.mkstemp(suffix=".zip")
        self._tmp_path = pathlib.Path(path)

        self._tmp_file = QtCore.QFile(str(self._tmp_path))
        if not self._tmp_file.open(QtCore.QIODevice.WriteOnly):
            self._cleanup_tmp()
            self.download_failed.emit("Failed to open temp file for download")
            return

        self._reply.readyRead.connect(self._on_ready_read)
        self._reply.downloadProgress.connect(self.download_progress.emit)
        self._reply.finished.connect(self._on_finished)
        self._reply.errorOccurred.connect(self._on_error)

    def cancel(self):
        if self._reply and self._reply.isRunning():
            self._reply.abort()

    @QtCore.Slot()
    def _on_ready_read(self):
        if not self._reply or not self._tmp_file:
            return
        data = self._reply.readAll()
        if data:
            self._tmp_file.write(data)

    @QtCore.Slot()
    def _on_error(self, _code):
        # handled in _on_finished to avoid double-emits
        pass

    @QtCore.Slot()
    def _on_finished(self):
        reply = self._reply
        self._reply = None

        if self._tmp_file:
            self._tmp_file.flush()
            self._tmp_file.close()

        if reply.error() == QtNetwork.QNetworkReply.OperationCanceledError:
            self._cleanup_tmp()
            self.download_cancelled.emit()
            reply.deleteLater()
            return

        if reply.error() != QtNetwork.QNetworkReply.NoError:
            msg = reply.errorString()
            self._cleanup_tmp()
            self.download_failed.emit(msg)
            reply.deleteLater()
            return

        status = reply.attribute(QtNetwork.QNetworkRequest.HttpStatusCodeAttribute)
        if status is not None and int(status) != 200:
            self._cleanup_tmp()
            self.download_failed.emit(f"HTTP {int(status)} downloading {self._am_url}")
            reply.deleteLater()
            return

        # Yay, everything is good! Well, at least we downloaded SOMETHING. Who knows what?
        try:
            zip_name = self._am_url.split("/")[-1]
            target_dir = pathlib.Path(FreeCAD.getUserAppDataDir()) / "Mod" / zip_name
            target_dir.mkdir(parents=True, exist_ok=True)

            with zipfile.ZipFile(str(self._tmp_path)) as zf:
                zf.extractall(str(target_dir))

            self.params.SetBool("am_downloaded", True)

            self._cleanup_tmp()
            self.download_complete.emit(target_dir)

        except Exception as e:
            self._cleanup_tmp()
            self.download_failed.emit(str(e))

        reply.deleteLater()
        QtCore.QCoreApplication.processEvents()  # Try to avoid Windows permissions errors on cleanup

    def _cleanup_tmp(self):
        if self._tmp_file and self._tmp_file.isOpen():
            self._tmp_file.close()
        self._tmp_file = None

        if self._tmp_path:
            try:
                self._tmp_path.unlink(missing_ok=True)
            except PermissionError:
                # Common on Windows, but doesn't really matter
                pass
            except OSError:
                pass
            self._tmp_path = None
