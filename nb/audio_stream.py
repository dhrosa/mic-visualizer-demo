import numpy as np

class AudioStream:
    def __init__(self, samples, fs, window_length):
        self.samples = samples
        self.fs = fs
        self.window_length = window_length
        n = (self.window_length // 2 + 1) if window_length % 2 == 0 else ((self.window_length + 1) // 2)
        self.freqs = np.linspace(0, self.fs / 2, n)

    def frame_stream(self):
        accum = []
        for s in self.samples:
            accum = np.append(accum, s)
            while len(accum) >= self.window_length:
                framed, accum = np.split(accum, [self.window_length])
                yield framed

    def psd_stream(self):
        from scipy import fft
        from pandas import Series
        # Real-valued FFT should have doubled energy everywhere except
        # at DC and Fs, which are not symmetric.
        sym = np.full_like(self.freqs, 2)
        sym[0] = 1
        sym[-1] = 1
        norm = sym / self.window_length
        for s in self.frame_stream():
            spectrum = np.abs(fft.rfft(s)) ** 2
            psd = spectrum * norm
            log_psd = np.log2(1+psd)
            yield Series(log_psd, index=self.freqs)
