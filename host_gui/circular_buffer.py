import numpy as np


class CircularBuffer:
    def __init__(self, width, height, fill_value=0):
        self._width = width
        self._height = height
        self._data = np.full((height, width), fill_value, dtype="float")
        self._col = 0

    def append(self, new_column):
        self._data[:, self._col] = new_column
        self._col = (self._col + 1) % self._width

    @property
    def buffers(self):
        older = self._data[:, self._col : self._width]
        newer = self._data[:, 0 : self._col]
        return older, newer
