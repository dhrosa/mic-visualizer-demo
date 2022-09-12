import numpy as np
import pandas as pd

def input_stream():
    import serial
    import binascii
    with serial.Serial("/dev/ttyACM0") as s:
        for line in s:
            raw = np.frombuffer(binascii.a2b_base64(line), dtype='h')
            yield raw

class AudioStream:
    def __init__(self, fs, window_length):
        self.fs = fs
        self.window_length = window_length
        self.freqs = np.linspace(0, self.fs / 2, (self.window_length // 2) + 1)

    def frame_stream(self):
        accum = []
        for s in input_stream():
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
