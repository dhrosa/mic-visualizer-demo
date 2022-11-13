from PySide6.QtCore import Qt, QLineF, QPointF
from PySide6.QtGui import QColor, QPainter, QPen
from PySide6.QtWidgets import QWidget

class Cursor(QWidget):
    def __init__(self, parent):
        super().__init__(parent)
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setVisible(False)

        self.target = QPointF(0, 0)

    def paintEvent(self, event):
        painter = QPainter(self)
        pen = QPen()
        pen.setDashPattern([10, 10])
        pen.setCapStyle(Qt.FlatCap)
        pen.setColor(QColor.fromHslF(0, 0, 0.5, 0.5))
        painter.setPen(pen)

        # Horizontal
        painter.drawLine(QLineF(0, self.target.y(),
                                self.width(), self.target.y()))
        # Vertical
        painter.drawLine(QLineF(self.target.x(), 0,
                                self.target.x(), self.height()))
