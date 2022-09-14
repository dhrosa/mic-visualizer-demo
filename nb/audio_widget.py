import itertools
import funcy
import numpy as np
from PySide6 import QtGui, QtWidgets, QtCore
from matplotlib import colors, cm
import matplotlib as mpl

from audio_stream import AudioStream

core = QtCore
gui = QtGui
widgets = QtWidgets
Qt = core.Qt

class AudioWidget(widgets.QWidget):
    def __init__(self, audio_stream):
        super().__init__()
        self.audio_stream = audio_stream

        self.row_count = len(self.audio_stream.freqs)
        self.col_count = 1024
        self.set_colormap_name('viridis')

        self.column_index = 0
        self.pixmap = gui.QPixmap(self.col_count, self.row_count)
        self.pixmap.fill()

    def sizeHint(self):
        return self.maximumSize()

    def paintEvent(self, event):
        painter = gui.QPainter(self)
        painter.setWindow(self.pixmap.rect())
        painter.drawPixmap(0, 0, self.pixmap)

    @property
    def colormap_name(self):
        return self.mapper.get_cmap().name

    def set_colormap_name(self, cmap_name):
        cmap = cm.get_cmap(cmap_name)
        norm = colors.BoundaryNorm(np.linspace(10, 34, 8), cmap.N)
        self.mapper = cm.ScalarMappable(norm, cmap)

    def logical_to_device_rect(self, rect):
        transform = gui.QTransform.fromScale(
            self.width() / self.pixmap.width(),
            self.height() / self.pixmap.height())
        return transform.mapRect(rect).toAlignedRect()

    def append_data(self, psd):
        self.setUpdatesEnabled(False)
        painter = gui.QPainter()
        painter.begin(self.pixmap)
        col = self.column_index
        pixel_colors = self.mapper.to_rgba(psd)
        for row, color in enumerate(reversed(pixel_colors)):
            painter.setPen(gui.QColor.fromRgbF(*color))
            painter.drawPoint(col, row)
        self.column_index += 1
        self.column_index %= self.col_count
        painter.end()
        self.setUpdatesEnabled(True)
        dirty_rect = self.logical_to_device_rect(core.QRectF(col, 0, 1, self.row_count))
        self.update(dirty_rect)



class MainWindow(widgets.QMainWindow):
    def __init__(self):
        super().__init__()

        import threading
        self.audio_stream = AudioStream(fs=24_000, window_length=2048)

        self.init_central_widget()
        self.init_options_dock()
        self.init_shortcuts()

        self.closing = False
        self.update_thread = threading.Thread(target=self.update_loop)
        self.update_thread.start()

        
    def init_central_widget(self):
        self.audio_widget = AudioWidget(self.audio_stream)
        central = widgets.QWidget()
        layout = widgets.QHBoxLayout(central)
        layout.addWidget(self.audio_widget)
        self.setCentralWidget(central)
        
        
    def init_options_dock(self):
        options_dock = widgets.QDockWidget("Spectrogram options")

        cmap_picker = widgets.QListWidget()
        cmap_picker.addItems(sorted(mpl.colormaps.keys()))
        cmap_picker.setCurrentItem(
            cmap_picker.findItems(self.audio_widget.colormap_name, Qt.MatchExactly)[0])
        cmap_picker.currentTextChanged.connect(self.audio_widget.set_colormap_name)
        options_dock.setWidget(cmap_picker)
        self.addDockWidget(Qt.RightDockWidgetArea, options_dock)

    def init_shortcuts(self):
        close_keys = [
            gui.QKeySequence.Close,
            gui.QKeySequence.Quit,
        ]
        for k in close_keys:
            gui.QShortcut(k, self, self.close)

    def closeEvent(self, event):
        self.closing = True
        self.update_thread.join()
        event.accept()

    def update_loop(self):
        stream = self.audio_stream.psd_stream()
        while not self.closing:
            self.audio_widget.append_data(next(stream))


