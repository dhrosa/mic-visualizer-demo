from math import floor
from threading import Lock
from PySide6.QtCore import QRectF, Signal, QPoint, QPointF
from PySide6.QtGui import QImage, QPainter, QTransform
from PySide6.QtWidgets import QWidget

from gui.cursor import Cursor


class ImageViewer(QWidget):
    binHovered = Signal(QPoint)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.image_lock = Lock()
        self.reset_image(1, 1)
        self.setMouseTracking(True)
        self.cursor = Cursor(self)

    def reset_image(self, width, height, fill_color=0xFF_00_00_00):
        self.image = QImage(width, height, QImage.Format_ARGB32)
        self.image.fill(fill_color)

    def update_logical_rect(self, rect):
        self.update(self.logical_to_widget_transform.mapRect(rect).toAlignedRect())

    @property
    def logical_to_widget_transform(self):
        return QTransform.fromScale(
            self.width() / self.image.width(), self.height() / self.image.height()
        )

    @property
    def widget_to_logical_transform(self):
        return QTransform.fromScale(
            self.image.width() / self.width(), self.image.height() / self.height()
        )

    def paintEvent(self, event):
        painter = QPainter(self)
        with self.image_lock:
            dest_rect = event.rect()
            source_rect = self.widget_to_logical_transform.mapRect(
                QRectF(dest_rect)
            ).toAlignedRect()
            painter.setWindow(self.image.rect())
            painter.drawImage(source_rect.topLeft(), self.image, source_rect)

    def moveEvent(self, event):
        self.update()
        super().moveEvent(event)

    def sizeHint(self):
        return self.image.size()

    def enterEvent(self, event):
        self.cursor.show()
        self.cursor.raise_()

    def mouseMoveEvent(self, event):
        widget_pos = QPointF(event.pos())
        image_pos = self.widget_to_logical_transform.map(widget_pos)
        # Each image pixel is mapped to a rectangle identified by its top-left
        # corner (towards origin).
        snapped_image_pos = QPoint(floor(image_pos.x()), floor(image_pos.y()))
        self.binHovered.emit(snapped_image_pos)

        # Snap cursor to midpoint of pixel rectangle.
        self.cursor.target = self.logical_to_widget_transform.map(
            QPointF(snapped_image_pos) + QPointF(0.5, 0.5)
        )
        self.cursor.setGeometry(0, 0, self.width(), self.height())
        self.cursor.update()
