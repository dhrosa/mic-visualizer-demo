import itertools

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
        # Need focus to get key events.
        self.setFocusPolicy(Qt.StrongFocus)
        # fixed = widgets.QSizePolicy.Fixed
        # self.setSizePolicy(fixed, fixed)

        self.row_count = len(self.audio_stream.freqs)
        self.col_count = 256
        self.set_colormap_name('viridis')

        self.column_index = 0
        self.pixmap = gui.QPixmap(self.col_count, self.row_count)
        self.pixmap.fill()

        pol = widgets.QSizePolicy.Expanding
        self.setSizePolicy(pol, pol)

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

    def append_data(self, psd):
        self.setUpdatesEnabled(False)
        painter = gui.QPainter()
        painter.begin(self.pixmap)
        col = self.column_index
        for row, color in enumerate(reversed(self.mapper.to_rgba(psd))):
            painter.setPen(gui.QColor.fromRgbF(*color))
            painter.drawPoint(col, row)
        self.column_index += 1
        self.column_index %= self.col_count
        painter.end()
        self.setUpdatesEnabled(True)
        self.update(col, 0, 1, self.row_count)



class MainWindow(widgets.QMainWindow):
    def __init__(self):
        super().__init__()

        import threading
        self.audio_stream = AudioStream(fs=24_000, window_length=1024)

        self.init_central_widget()
        self.init_options_dock()

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

    def closeEvent(self, event):
        self.closing = True
        self.update_thread.join()
        event.accept()


    def keyPressEvent(self, event):
        if event.modifiers() == Qt.NoModifier:
            self.close()

    def update_loop(self):
        stream = self.audio_stream.psd_stream()
        while not self.closing:
            self.audio_widget.append_data(next(stream))


