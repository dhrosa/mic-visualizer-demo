from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QImage, QPixmap
from PySide6.QtWidgets import QComboBox
import numpy as np
from funcy import memoize

from image import image_numpy_view, cmap_to_lut, unit_linspace


@memoize
def _horizontal_gradient(width, height):
    return np.broadcast_to(unit_linspace(width), (height, width))


# Sequential colormaps from
# https://matplotlib.org/stable/tutorials/colors/colormaps.html#classes-of-colormaps
_cmap_names = [
    "viridis",
    "plasma",
    "inferno",
    "magma",
    "cividis",
    "Greys",
    "Purples",
    "Blues",
    "Greens",
    "Oranges",
    "Reds",
    "YlOrBr",
    "YlOrRd",
    "OrRd",
    "PuRd",
    "RdPu",
    "BuPu",
    "GnBu",
    "PuBu",
    "YlGnBu",
    "PuBuGn",
    "BuGn",
    "YlGn",
    "binary",
    "gist_yarg",
    "gist_gray",
    "gray",
    "bone",
    "pink",
    "spring",
    "summer",
    "autumn",
    "winter",
    "cool",
    "Wistia",
    "hot",
    "afmhot",
    "gist_heat",
    "copper",
]


class ColormapPicker(QComboBox):
    def __init__(self, original_name):
        super().__init__()
        self.setIconSize(QSize(256, self.fontMetrics().height()))
        for name in _cmap_names:
            self.addItem(self.preview_icon(name), name)
        self.setCurrentText(original_name)
        self.previous_index = self.currentIndex()

    def preview_icon(self, name):
        width = self.iconSize().width()
        height = self.iconSize().height()
        image = QImage(width, height, QImage.Format_ARGB32)
        dest = image_numpy_view(image)

        lut = cmap_to_lut(name, n=width)
        lut.Map(0, 1, _horizontal_gradient(width, height), dest)

        return QIcon(QPixmap.fromImage(image))

    def showPopup(self):
        self.previous_index = self.currentIndex()
        super().showPopup()

    def hidePopup(self):
        super().hidePopup()
        self.setCurrentIndex(self.previous_index)
        self.currentTextChanged.emit(self.currentText())
