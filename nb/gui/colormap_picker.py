from PySide6.QtCore import QSize
from PySide6.QtGui import QIcon, QImage, QPixmap
from PySide6.QtWidgets import QComboBox
from matplotlib import colormaps
import numpy as np
from funcy import memoize

from image import image_numpy_view, cmap_to_lut, unit_linspace

@memoize
def _horizontal_gradient(width, height):
    return np.broadcast_to(unit_linspace(width), (height, width))

class ColormapPicker(QComboBox):
    def __init__(self, original_name):
        super().__init__()
        self.setIconSize(QSize(256, self.fontMetrics().height()))
        for name in colormaps.keys():
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
