from PySide6.QtCore import Qt, QKeyCombination, QLineF, QObject, QPoint, QPointF, QRect, QRectF, QSize, Signal
from PySide6.QtGui import QColor, QIcon, QImage, QKeySequence, QPainter, QPen, QPixmap, QShortcut, QTransform
from PySide6.QtWidgets import QApplication, QComboBox, QDialog, QDialogButtonBox, QDockWidget, QFormLayout, QLabel, QMainWindow, QRubberBand, QScrollArea, QVBoxLayout, QWidget

import itertools
import funcy
import numpy as np
from math import floor

from matplotlib import colors, cm
import matplotlib as mpl
from si_prefix import si_format
from threading import Thread, Lock
from collections import deque

from audio_stream import IterThread, Broadcaster, AudioStream, serial_samples

class ImageBuffer:
    """
    Presents a numpy-compatible view of a QImage in
    (row, column, channel) order.
    """
    def __init__(self, image):
        self.image = image

    @property
    def __array_interface__(self):
        return {
            'typestr': '<u1',
            'data': self.image.bits(),
            'shape': (self.image.height(), self.image.width(), 4),
            'strides': (self.image.bytesPerLine(), 4, 1),
        }


def image_numpy_view(image):
    return np.array(ImageBuffer(image), copy=False)


class Cursor(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setVisible(False)

        self.target = QPointF(0, 0)

    def paintEvent(self, event):
        painter = QPainter(self)
        pen = QPen()
        pen.setDashPattern([10, 10])
        pen.setCapStyle(Qt.FlatCap)
        pen.setColor(QColor.fromHslF(0, 0, 0.5, 0.5))
        painter.setPen(pen)

        # Horizontal
        painter.drawLine(QLineF(0, self.target.y(),
                                self.width(), self.target.y()))
        # Vertical
        painter.drawLine(QLineF(self.target.x(), 0,
                                self.target.x(), self.height()))

class ColormapPicker(QComboBox):
    def __init__(self, original_name):
        super().__init__()
        self.setIconSize(QSize(256, self.fontMetrics().height()))
        for name in mpl.colormaps.keys():
            self.addItem(self.preview_icon(name), name)
        self.setCurrentText(original_name)
        self.previous_index = self.currentIndex()

    def preview_icon(self, name):
        cmap = cm.get_cmap(name)
        image = QImage(self.iconSize().width(), 1, QImage.Format_ARGB32)
        data = image.bits()
        colors = cmap(np.linspace(0, 1, self.iconSize().width()), bytes=True)
        for col, (r, g, b, a) in enumerate(colors):
            i = 4 * col
            data[i:i+4] = bytes([b, g, r, a])
        return QIcon(QPixmap.fromImage(image.scaled(self.iconSize())))

    def showPopup(self):
        self.previous_index = self.currentIndex()
        super().showPopup()

    def hidePopup(self):
        super().hidePopup()
        self.setCurrentIndex(self.previous_index)
        self.currentTextChanged.emit(self.currentText())


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
        self.scale = 1
        self.setSizeAdjustPolicy(QScrollArea.AdjustToContentsOnFirstShow)
        QShortcut(QKeySequence.ZoomIn, self, self.zoom_in)
        QShortcut(QKeySequence.ZoomOut, self, self.zoom_out)

    def zoom(self, factor):
        self.scale *= factor
        old_size = self.widget().size()
        new_size = self.widget().sizeHint() * self.scale
        self.widget().resize(new_size)

    def zoom_in(self):
        self.zoom(2)

    def zoom_out(self):
        self.zoom(0.5)


class AudioWidget(QMainWindow):
    def __init__(self, samples, fs, window_length):
        super().__init__()
        self.audio = AudioStream(samples, fs, window_length)

        self.row_count = len(self.audio.freqs)
        self.col_count = 1024
        self.column_index = 0
        self.data = deque(maxlen=self.col_count)

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
        tool_bar.addWidget(colormap_picker)
        colormap_picker.currentTextChanged.connect(self.set_colormap_name)
        colormap_picker.textHighlighted.connect(self.set_colormap_name)


    def update_statusbar(self, bin_pos):
        f = self.audio.freqs[self.row_count - bin_pos.y() - 1]
        f_str = si_format(f, precision=1, format_str="{value}{prefix}Hz")
        self.frequency_label.setText(f"F={int(round(f)):5d} Hz")


    def process_audio(self):
        for psd in self.audio.psd_stream():
            with self.viewer.image_lock:
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


    @funcy.print_durations
    def set_colormap_name(self, cmap_name):
        cmap = cm.get_cmap(cmap_name)
        norm = colors.BoundaryNorm(np.linspace(10, 34, 8), cmap.N)
        self.mapper = cm.ScalarMappable(norm, cmap)
        # Redraw existing data with new colors
        self.column_index = 0
        data = self.data.copy()
        self.data.clear()
        with self.viewer.image_lock:
            for psd in data:
                self.append_data(psd)


    def append_data(self, psd):
        self.data.append(psd)
        col = self.column_index
        # shape = psd.shape + (4,)
        pixel_colors = np.array(self.mapper.to_rgba(psd, bytes=True), dtype='u8')
        # RGBA -(roll)-> ARGB -(flip)-> BGRA
        pixel_colors = np.flip(
            np.roll(pixel_colors, 1, axis=-1),
            axis=-1)
        data = image_numpy_view(self.viewer.image)
        data[:, col, :] = np.flip(pixel_colors, axis=0)
        self.column_index += 1
        self.column_index %= self.col_count
        self.viewer.update_logical_rect(QRectF(col, 0, 1, self.row_count))


class Context(QObject):
    def __init__(self, app):
        super().__init__()
        self.app = app
        self.fs = 24_000
        self.broadcaster = Broadcaster()
        self.broadcast_thread = IterThread(self.broadcast_loop())
        self.windows = set()
        self.app.aboutToQuit.connect(self.broadcast_thread.close)
        #self.new_window_prompt()
        self.new_window(1024)

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
