from PySide6 import QtGui, QtWidgets, QtCore, QtOpenGLWidgets
import sys
import time

import pandas as pd
import numpy as np
import funcy
import more_itertools
from typing import cast

core = QtCore
gui = QtGui
widgets = QtWidgets
gl_widgets = QtOpenGLWidgets

def spy(gen):
    (first,), gen = more_itertools.spy(gen, 1)
    return first, gen

def print_bounds(name, stream):
    first, stream = spy(stream)
    cum_min = np.amin(first)
    cum_max = np.amax(first)
    for s in stream:
        cum_min = min(cum_min, np.amin(s))
        cum_max = max(cum_max, np.amax(s))
        print(f"{name}: {(cum_min, cum_max)}")
        yield s

class AudioStream:
    def __init__(self, fs, window_length):
        self.fs = fs
        self.window_length = window_length
        self.freqs = np.linspace(0, self.fs / 2, (self.window_length // 2) + 1)

    def input_stream(self):
        import serial
        import binascii
        with serial.Serial("/dev/ttyACM0") as s:
            for line in s:
                raw = np.frombuffer(binascii.a2b_base64(line), dtype='h')
                yield raw

    def frame_stream(self):
        accum = []
        for s in self.input_stream():
            accum = np.append(accum, s)
            while len(accum) >= self.window_length:
                framed, accum = np.split(accum, [self.window_length])
                yield framed

    def psd_stream(self):
        # Real-valued FFT should have doubled energy everywhere except
        # at DC and Fs, which are not symmetric.
        sym = np.full_like(self.freqs, 2)
        sym[0] = 1
        sym[-1] = 1
        norm = sym / self.window_length
        for s in self.frame_stream():
            spectrum = np.abs(np.fft.rfft(s)) ** 2
            psd = spectrum * norm
            log_psd = np.log2(1+psd)
            yield pd.Series(log_psd, index=self.freqs)

@funcy.decorator
def event_handler(call, event_subclass):
    call.event = cast(event_subclass, call.event)
    ret = call()
    if ret is None:
        return False
    return ret
            
class Viewer(core.QObject):
    have_new_data = core.Signal()
    
    def __init__(self):
        super().__init__()
        import threading
        
        self.audio = AudioStream(fs=24_000, window_length=512)
        self.scene = widgets.QGraphicsScene()
        self.scene.setBackgroundBrush(gui.QColor('aliceblue'))
        self.view = widgets.QGraphicsView()

        # self.gl_widget = gl_widgets.QOpenGLWidget()
        # self.view.setViewport(self.gl_widget)

        self.row_count = len(self.audio.freqs)
        self.col_count = self.row_count
        self.columns = []
        self.column_index = 0
        self.scene.setSceneRect(0, 0, self.col_count, self.row_count)
        self.view.setHorizontalScrollBarPolicy(core.Qt.ScrollBarAlwaysOff)
        self.view.setVerticalScrollBarPolicy(core.Qt.ScrollBarAlwaysOff)
        self.view.setScene(self.scene)
        self.view.fitInView(self.scene.sceneRect(), core.Qt.KeepAspectRatio)
        self.scene.changed.connect(self.view.updateScene)
        self.view.show()
        self.view.installEventFilter(self)

        self.closing = False
        self.have_new_data.connect(self.render_new_data)
        self.update_thread = threading.Thread(target=self.update_loop)
        self.update_thread.start()

    def eventFilter(self, obj: core.QObject, event: core.QEvent):
        handlers = {
            core.QEvent.KeyPress: self.key_event,
            core.QEvent.Wheel: self.wheel_event,
            core.QEvent.Paint: self.paint_event,
        }
        handler = handlers.get(event.type(), None)
        if handler is None:
            return super().eventFilter(obj, event)
        handler(event)
        return True

    @event_handler(gui.QKeyEvent)
    def key_event(self, event):
        if event.modifiers() == core.Qt.NoModifier:
            self.view.close()

    @event_handler(gui.QWheelEvent)
    def wheel_event(self, event):
        print(event)

    @event_handler(gui.QPaintEvent)
    def paint_event(self, event):
        print(event)
        return False
    
    def close(self):
        self.closing = True
        self.update_thread.join()

    def maybe_add_column(self, c):
        if c < len(self.columns):
            return
        col = []
        pen = gui.QPen(gui.Qt.NoPen)
        for r in reversed(range(self.row_count)):
            rect = self.scene.addRect(0, 0, 1, 1, pen)
            rect.setPos(c, r)
            col.append(rect)
        self.columns.append(col)

    def update_loop(self):
        from matplotlib import colors
        from matplotlib import cm
        cmap = cm.get_cmap('viridis')
        norm = colors.BoundaryNorm(np.linspace(10, 34, 8), cmap.N)
        mapper = cm.ScalarMappable(norm, 'viridis')
        stream = self.audio.psd_stream()        
        while not self.closing:
            s = next(stream)
            self.new_data = mapper.to_rgba(s)
            self.have_new_data.emit()

    def render_new_data(self):
        self.maybe_add_column(self.column_index)
        column = self.columns[self.column_index]
        for rect, color in zip(column, self.new_data):
            rect.setBrush(gui.QBrush(gui.QColor.fromRgbF(*color)))
        self.column_index += 1
        self.column_index %= self.col_count
            

def main():
    app = widgets.QApplication([])
    
    viewer = Viewer()    
    app.lastWindowClosed.connect(viewer.close)
    sys.exit(app.exec())

main()
