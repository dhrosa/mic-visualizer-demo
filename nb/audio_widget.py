from PySide6.QtCore import Qt, QObject, QRectF
from PySide6.QtGui import QColor, QImage, QKeySequence, QPainter, QShortcut, QTransform
from PySide6.QtWidgets import QApplication, QWidget

import itertools
import funcy
import numpy as np

from matplotlib import colors, cm
import matplotlib as mpl
from threading import Thread

from audio_stream import IterThread, Broadcaster, AudioStream, serial_samples

class AudioWidget(QWidget):
    def __init__(self, samples, fs, window_length):
        super().__init__()
        self.audio = AudioStream(samples, fs, window_length)

        self.row_count = len(self.audio.freqs)
        self.col_count = 1024
        self.set_colormap_name('viridis')

        self.column_index = 0
        self.image = QImage(self.col_count, self.row_count, QImage.Format_RGB32)
        self.image.fill(0xFF000000)

        self.update_thread = IterThread(self.process_audio())


    def process_audio(self):
        for psd in self.audio.psd_stream():
            self.append_data(psd)
            yield


    def sizeHint(self):
        return self.maximumSize()


    def closeEvent(self, event):
        self.update_thread.close()
        event.accept()


    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setWindow(self.image.rect())
        painter.drawImage(0, 0, self.image)


    @property
    def colormap_name(self):
        return self.mapper.get_cmap().name


    def set_colormap_name(self, cmap_name):
        cmap = cm.get_cmap(cmap_name)
        norm = colors.BoundaryNorm(np.linspace(10, 34, 8), cmap.N)
        self.mapper = cm.ScalarMappable(norm, cmap)


    def logical_to_device_rect(self, rect):
        transform = QTransform.fromScale(
            self.width() / self.image.width(),
            self.height() / self.image.height())
        return transform.mapRect(rect).toAlignedRect()


    def append_data(self, psd):
        col = self.column_index
        pixel_colors = self.mapper.to_rgba(psd, bytes=True, alpha=True)
        data = self.image.bits()
        stride = self.image.bytesPerLine()
        i = col * 4
        for row, (r, g, b, a) in enumerate(reversed(pixel_colors)):
            data[i:i+3] = bytes([b, g, r])
            i += stride
        self.column_index += 1
        self.column_index %= self.col_count
        dirty_rect = self.logical_to_device_rect(QRectF(col, 0, 1, self.row_count))
        self.update(dirty_rect)


class Context(QObject):
    def __init__(self, app):
        super().__init__()
        self.app = app
        self.fs = 24_000
        self.broadcaster = Broadcaster()
        self.broadcast_thread = IterThread(self.broadcast_loop())
        self.windows = []
        self.app.lastWindowClosed.connect(self.broadcast_thread.close)
        self.new_window()


    def broadcast_loop(self):
        for samples in serial_samples():
            self.broadcaster.broadcast(samples)
            yield


    def new_window(self):
        w = AudioWidget(self.broadcaster.subscribe(), self.fs, 2048)
        QShortcut(QKeySequence.New, w, self.new_window)
        QShortcut(QKeySequence.Close, w, w.close)
        QShortcut(QKeySequence.Quit, w, self.app.closeAllWindows)
        self.windows.append(w)
        w.show()
