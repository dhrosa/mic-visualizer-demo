import numpy as np
import pandas as pd
from contextlib import contextmanager
from threading import Lock, Thread
from queue import Queue
from inspect import isgenerator, isgeneratorfunction

class IterThread(Thread):
    def __init__(self, gen):
        super().__init__()
        if isgeneratorfunction(gen):
            gen = gen()
        if not isgenerator(gen):
            raise TypeError("`gen` must be a generator or generator function.")
        self.gen = gen
        self.closing = False
        self.start()

    def __del__(self):
        self.close()

    def close(self):
        self.closing = True
        self.join()

    def run(self):
        while not self.closing:
            next(self.gen)

class Broadcaster:
    def __init__(self):
        self.sinks_lock = Lock()
        self.sinks = {}
        self.next_sink_id = 0

    def broadcast(self, value):
        with self.sinks_lock:
            sinks = tuple(self.sinks.values())
            for sink in sinks:
                sink.put(value)

    def subscribe(self):
        sink = Queue()
        with self.sinks_lock:
            sink_id = self.next_sink_id
            self.next_sink_id += 1
            self.sinks[sink_id] = sink
        try:
            while True:
                item = sink.get()
                if item is None:
                    break
                yield item
        finally:
            with self.sinks_lock:
                del self.sinks[sink_id]


def serial_samples():
    import numpy as np
    import serial
    import binascii
    with serial.Serial("/dev/ttyACM0") as s:
        for line in s:
            yield np.frombuffer(binascii.a2b_base64(line), dtype='h')


def simulated_samples():
    from scipy.io import wavfile
    import time
    fs, data = wavfile.read('cardinal.wav')
    mono = data.mean(axis=1)
    n = len(mono)
    chunk_size = 1000

    while True:
        for offset in range(0, n, chunk_size):
            samples = mono[offset:min(n, offset+chunk_size)]
            yield samples
            time.sleep(len(samples) / fs)



class AudioStream:
    def __init__(self, samples, fs, window_length):
        self.samples = samples
        self.fs = fs
        self.window_length = window_length
        self.freqs = np.linspace(0, self.fs / 2, (self.window_length // 2) + 1)

    def frame_stream(self):
        accum = []
        for s in self.samples:
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
