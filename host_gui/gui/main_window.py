from PySide6.QtWidgets import QApplication, QCheckBox, QLabel, QMainWindow
import numpy as np
from si_prefix import si_format

from lut import Table
from audio_stream import AudioStream
from iter_thread import IterThread
from gui.cursor import Cursor
from gui.colormap_picker import ColormapPicker
from gui.image_viewer import ImageViewer
from gui.scroll_area import ScrollArea
from image import cmap_to_lut, image_numpy_view
from circular_buffer import CircularBuffer

def _largest_screen_size():
    app = QApplication.instance()
    sizes = [screen.size() for screen in app.screens()]
    return max(sizes, key=lambda s: s.width() * s.height())


class MainWindow(QMainWindow):
    def __init__(self, samples, fs, window_length):
        super().__init__()
        self.audio = AudioStream(samples, fs, window_length)
        self.row_count = len(self.audio.freqs)
        self.col_count = _largest_screen_size().width()
        self.data = CircularBuffer(self.col_count, self.row_count, 11)

        self.viewer = ImageViewer()
        self.viewer.reset_image(self.col_count, self.row_count)
        self.viewer.binHovered.connect(self.update_statusbar)
        self.scroll_area = ScrollArea()
        self.scroll_area.setWidget(self.viewer)
        self.setCentralWidget(self.scroll_area)

        self.set_colormap_name('viridis')
        self.init_status_bar()
        self.init_toolbar()
        self.update_thread = IterThread(self.process_audio())


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
