from funcy import memoize
import numpy as np
from matplotlib.cm import get_cmap

from lut import Table


class ImageBuffer:
    """
    Presents a numpy-compatible view of a QImage in
    (row, column) order as uint32 values.
    """

    def __init__(self, image):
        self.image = image

    @property
    def __array_interface__(self):
        return {
            "typestr": "<u4",
            "data": self.image.bits(),
            "shape": (self.image.height(), self.image.width()),
            "strides": (self.image.bytesPerLine(), 4),
        }


def image_numpy_view(image):
    return np.array(ImageBuffer(image), copy=False)


@memoize
def unit_linspace(n):
    return np.linspace(0, 1, n, endpoint=True)


@memoize
def cmap_to_lut(cmap_name, n):
    cmap = get_cmap(cmap_name)
    return Table(cmap(unit_linspace(n), bytes=True, alpha=1))
