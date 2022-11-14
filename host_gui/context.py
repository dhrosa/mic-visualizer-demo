from PySide6.QtCore import QObject
from PySide6.QtGui import QKeySequence, QShortcut
from broadcaster import Broadcaster
from iter_thread import IterThread
from sample_source import simulated_samples
from gui.main_window import MainWindow


class Context(QObject):
    def __init__(self, app):
        super().__init__()
        self.app = app
        self.fs = 24_000
        self.broadcaster = Broadcaster()
        self.broadcast_thread = IterThread(self.broadcast_loop())
        self.windows = set()
        self.app.aboutToQuit.connect(self.broadcast_thread.close)
        self.new_window(1024)

    def broadcast_loop(self):
        for samples in simulated_samples():
            self.broadcaster.broadcast(samples)
            yield

    def new_window(self, window_length):
        w = MainWindow(self.broadcaster.subscribe(), self.fs, window_length)
        QShortcut(QKeySequence.Close, w, w.close)
        QShortcut(QKeySequence.Quit, w, self.app.closeAllWindows)
        self.windows.add(w)
        w.show()
