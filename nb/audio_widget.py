from PySide6.QtCore import Qt, QObject, QRectF, QSize
from PySide6.QtGui import QColor, QIcon, QImage, QKeySequence, QPainter, QPixmap, QShortcut, QTransform
from PySide6.QtWidgets import QApplication, QComboBox, QDialog, QDialogButtonBox, QFormLayout, QLabel, QVBoxLayout, QWidget

import itertools
import funcy
import numpy as np
from weakref import WeakSet

from matplotlib import colors, cm
import matplotlib as mpl
from si_prefix import si_format
from threading import Thread, Lock
from collections import deque

from audio_stream import IterThread, Broadcaster, AudioStream, serial_samples

class NewWindowDialog(QDialog):
    def __init__(self, fs):
        super().__init__()
        layout = QFormLayout(self)
        layout.addRow("Sample Rate", QLabel(str(fs)))

        self.window_length = QComboBox()
        default_value = 1024
        default_index = 0
        for i, power in enumerate(range(8, 16)):
            val = 1 << power
            if val == default_value:
                default_index = i
            duration = si_format(val / fs, precision=1)
            freq = si_format(fs / val, precision=1)
            text = f'{val} samples / {duration}s / {freq}Hz)'
            self.window_length.addItem(text, val)
        self.window_length.setCurrentIndex(default_index)
        layout.addRow("FFT window length", self.window_length)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

class ColormapDialog(QDialog):
    def __init__(self, parent, original_name):
        super().__init__(parent)
        self.icon_size = QSize(256, 32)

        layout = QVBoxLayout(self)
        self.colormap_name = QComboBox()
        self.colormap_name.setIconSize(self.icon_size)
        for name in mpl.colormaps.keys():
            self.colormap_name.addItem(self.preview_icon(name), name)
        self.colormap_name.setCurrentText(original_name)
        layout.addWidget(self.colormap_name)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def preview_icon(self, name):
        cmap = cm.get_cmap(name)
        image = QImage(self.icon_size.width(), 1, QImage.Format_ARGB32)
        data = image.bits()
        colors = cmap(np.linspace(0, 1, self.icon_size.width()), bytes=True)
        for col, (r, g, b, a) in enumerate(colors):
            i = 4 * col
            data[i:i+4] = bytes([b, g, r, a])
        return QIcon(QPixmap.fromImage(image.scaled(self.icon_size)))


class ImageViewer(QWidget):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.image_lock = Lock()
        self.reset_image(1, 1)

    def reset_image(self, width, height, fill_color=0xFF_00_00_00):
        self.image = QImage(width, height, QImage.Format_ARGB32)
        self.image.fill(fill_color)

    def update_logical_rect(self, rect):
        self.update(self.logical_to_device_rect(rect))

    def logical_to_device_rect(self, rect):
        transform = QTransform.fromScale(
            self.width() / self.image.width(),
            self.height() / self.image.height())
        return transform.mapRect(rect).toAlignedRect()

    def paintEvent(self, event):
        painter = QPainter(self)
        with self.image_lock:
            painter.setWindow(self.image.rect())
            painter.drawImage(0, 0, self.image)


class AudioWidget(ImageViewer):
    def __init__(self, samples, fs, window_length):
        super().__init__()
        self.audio = AudioStream(samples, fs, window_length)

        self.row_count = len(self.audio.freqs)
        self.col_count = 1024
        self.data = deque(maxlen=self.col_count)
        self.set_colormap_name('viridis')

        self.column_index = 0
        self.reset_image(self.col_count, self.row_count)

        self.init_shortcuts()
        self.update_thread = IterThread(self.process_audio())


    def init_shortcuts(self):
        QShortcut(QKeySequence("c"), self, self.colormap_prompt)


    def colormap_prompt(self):
        def on_complete(dialog):
            self.set_colormap_name(dialog.colormap_name.currentText())
        dialog = ColormapDialog(self, self.colormap_name)
        dialog.accepted.connect(lambda: on_complete(dialog))
        dialog.open()


    def process_audio(self):
        for psd in self.audio.psd_stream():
            with self.image_lock:
                self.append_data(psd)
            yield

    def sizeHint(self):
        return self.maximumSize()


    def closeEvent(self, event):
        self.update_thread.close()
        event.accept()


    @property
    def colormap_name(self):
        return self.mapper.get_cmap().name


    def set_colormap_name(self, cmap_name):
        cmap = cm.get_cmap(cmap_name)
        norm = colors.BoundaryNorm(np.linspace(10, 34, 8), cmap.N)
        self.mapper = cm.ScalarMappable(norm, cmap)
        # Redraw existing data with new colors
        self.column_index = 0
        data = self.data.copy()
        self.data.clear()
        with self.image_lock:
            for psd in data:
                self.append_data(psd)


    def append_data(self, psd):
        self.data.append(psd)
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
        self.update_logical_rect(QRectF(col, 0, 1, self.row_count))


class Context(QObject):
    def __init__(self, app):
        super().__init__()
        self.app = app
        self.fs = 24_000
        self.broadcaster = Broadcaster()
        self.broadcast_thread = IterThread(self.broadcast_loop())
        self.windows = set()
        self.app.aboutToQuit.connect(self.broadcast_thread.close)
        self.new_window_prompt()

    def new_window_prompt(self):
        dialog = NewWindowDialog(self.fs)
        dialog.accepted.connect(lambda: self.new_window_prompt_complete(dialog))
        dialog.open()

    def new_window_prompt_complete(self, dialog):
        self.new_window(dialog.window_length.currentData())

    def broadcast_loop(self):
        for samples in serial_samples():
            self.broadcaster.broadcast(samples)
            yield


    def new_window(self, window_length):
        w = AudioWidget(self.broadcaster.subscribe(), self.fs, window_length)
        QShortcut(QKeySequence.New, w, self.new_window_prompt)
        QShortcut(QKeySequence.Close, w, w.close)
        QShortcut(QKeySequence.Quit, w, self.app.closeAllWindows)
        self.windows.add(w)
        w.show()
