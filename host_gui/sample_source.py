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
