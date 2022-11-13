from PySide6.QtCore import Qt, QKeyCombination, QLineF, QObject, QPoint, QPointF, QRect, QRectF, QSize, Signal
from PySide6.QtGui import QColor, QIcon, QImage, QKeySequence, QPainter, QPen, QPixmap, QShortcut, QTransform
from PySide6.QtWidgets import QApplication, QCheckBox, QComboBox, QDialog, QDialogButtonBox, QDockWidget, QFormLayout, QLabel, QMainWindow, QRubberBand, QScrollArea, QVBoxLayout, QWidget

import itertools
import funcy
import numpy as np
from scipy import fft
from math import floor

from matplotlib import colors, cm
import matplotlib as mpl
from si_prefix import si_format
from threading import Thread, Lock
from collections import deque

from audio_stream import IterThread, Broadcaster, AudioStream, serial_samples, simulated_samples
from lut import Table

from gui.cursor import Cursor
from gui.colormap_picker import ColormapPicker
from image import cmap_to_lut, image_numpy_view


class ImageViewer(QWidget):
    binHovered = Signal(QPoint)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.image_lock = Lock()
        self.reset_image(1, 1)
        self.setMouseTracking(True)
        self.cursor = Cursor(self)


    def reset_image(self, width, height, fill_color=0xFF_00_00_00):
        self.image = QImage(width, height, QImage.Format_ARGB32)
        self.image.fill(fill_color)


    def update_logical_rect(self, rect):
        self.update(self.logical_to_widget_transform.mapRect(rect).toAlignedRect())


    @property
    def logical_to_widget_transform(self):
        return QTransform.fromScale(
            self.width() / self.image.width(),
            self.height() / self.image.height())


    @property
    def widget_to_logical_transform(self):
        return QTransform.fromScale(
            self.image.width() / self.width(),
            self.image.height() / self.height())


    def paintEvent(self, event):
        painter = QPainter(self)
        with self.image_lock:
            dest_rect = event.rect()
            source_rect = self.widget_to_logical_transform.mapRect(QRectF(dest_rect)).toAlignedRect()
            painter.setWindow(self.image.rect())
            painter.drawImage(source_rect.topLeft(), self.image, source_rect)


    def moveEvent(self, event):
        self.update()
        super().moveEvent(event)


    def sizeHint(self):
        return self.image.size()


    def enterEvent(self, event):
        self.cursor.show()
        self.cursor.raise_()


    def mouseMoveEvent(self, event):
        widget_pos = QPointF(event.pos())
        image_pos = self.widget_to_logical_transform.map(widget_pos)
        # Each image pixel is mapped to a rectangle identified by its top-left
        # corner (towards origin).
        snapped_image_pos = QPoint(floor(image_pos.x()), floor(image_pos.y()))
        self.binHovered.emit(snapped_image_pos)

        # Snap cursor to midpoint of pixel rectangle.
        self.cursor.target = self.logical_to_widget_transform.map(
            QPointF(snapped_image_pos) + QPointF(0.5, 0.5))
        self.cursor.setGeometry(0, 0, self.width(), self.height())
        self.cursor.update()



class ScrollArea(QScrollArea):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._scale = 1
        self.set_fit_to_window(True)
        QShortcut(QKeySequence.ZoomIn, self, self.zoom_in)
        QShortcut(QKeySequence.ZoomOut, self, self.zoom_out)

    def set_fit_to_window(self, fit):
        self._fit = fit
        self.setWidgetResizable(fit)
        self.setSizeAdjustPolicy(QScrollArea.AdjustToContents if fit else QScrollArea.AdjustIgnored)
        
    def zoom(self, factor):
        self._scale *= factor
        old_size = self.widget().size()
        new_size = self.widget().sizeHint() * _self.scale
        self.widget().resize(new_size)

    def zoom_in(self):
        self.zoom(2)

    def zoom_out(self):
        self.zoom(0.5)


class RotatingData:
    def __init__(self, width, height, fill_value=0):
        self._width = width
        self._height = height
        self._data = np.full((height, width), fill_value, dtype='float')
        self._col = 0

    def append(self, new_column):
        self._data[:, self._col] = new_column
        self._col = (self._col + 1) % self._width

    @property
    def buffers(self):
        older = self._data[:, self._col:self._width]
        newer = self._data[:, 0:self._col]
        return older, newer

def largest_screen_size():
    app = QApplication.instance()
    sizes = [screen.size() for screen in app.screens()]
    return max(sizes, key=lambda s: s.width() * s.height())

class AudioWidget(QMainWindow):
    def __init__(self, samples, fs, window_length):
        super().__init__()
        screen_size = largest_screen_size()
        self.audio = AudioStream(samples, fs, window_length)        
        self.row_count = len(self.audio.freqs)
        self.col_count = screen_size.width()
        self.data = RotatingData(self.col_count, self.row_count, 11)

        self.viewer = ImageViewer()
        self.viewer.reset_image(self.col_count, self.row_count)
        self.viewer.binHovered.connect(self.update_statusbar)
        self.scroll_area = ScrollArea()
        self.scroll_area.setWidget(self.viewer)
        self.setCentralWidget(self.scroll_area)

        self.set_colormap_name('viridis')
        self.init_shortcuts()
        self.init_status_bar()
        self.init_toolbar()
        self.update_thread = IterThread(self.process_audio())


    def init_shortcuts(self):
        pass


    def init_status_bar(self):
        self.frequency_label = QLabel()
        self.statusBar().addPermanentWidget(self.frequency_label)

        font = self.frequency_label.font()
        font.setPointSize(24)
        font.setFamily("monospace")
        self.frequency_label.setFont(font)


    def init_toolbar(self):
        tool_bar = self.addToolBar("Tool Bar")
        tool_bar.setFloatable(False)

        colormap_picker = ColormapPicker(self.colormap_name)
        colormap_picker.currentTextChanged.connect(self.set_colormap_name)
        colormap_picker.textHighlighted.connect(self.set_colormap_name)
        tool_bar.addWidget(colormap_picker)

        fit_button = QCheckBox("Fit To Window")
        tool_bar.addWidget(fit_button)
        


    def update_statusbar(self, bin_pos):
        f = self.audio.freqs[self.row_count - bin_pos.y() - 1]
        f_str = si_format(f, precision=1, format_str="{value}{prefix}Hz")
        self.frequency_label.setText(f"F={int(round(f)):5d} Hz")


    def process_audio(self):
        for psd in self.audio.psd_stream():
            self.data.append(psd)
            self.render()
            yield

    def render(self):
        older, newer = self.data.buffers
        split_point = older.shape[1]

        with self.viewer.image_lock:
            out = image_numpy_view(self.viewer.image)
            self.table.Map(10, 34, np.flipud(older), out[:, 0:split_point])
            self.table.Map(10, 34, np.flipud(newer), out[:, split_point:])

        self.viewer.update()

    def sizeHint(self):
        return self.maximumSize()


    def closeEvent(self, event):
        self.update_thread.close()
        event.accept()


    @property
    def colormap_name(self):
        return self._cmap_name


    def set_colormap_name(self, cmap_name):
        self._cmap_name = cmap_name
        self.table = cmap_to_lut(cmap_name, 256)
        self.render()


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
        w = AudioWidget(self.broadcaster.subscribe(), self.fs, window_length)
        QShortcut(QKeySequence.Close, w, w.close)
        QShortcut(QKeySequence.Quit, w, self.app.closeAllWindows)
        self.windows.add(w)
        w.show()
