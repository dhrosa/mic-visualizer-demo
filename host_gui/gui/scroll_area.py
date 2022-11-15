from PySide6.QtGui import QKeySequence, QShortcut
from PySide6.QtWidgets import QScrollArea


class ScrollArea(QScrollArea):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._scale = 1
        self.set_fit_to_window(True)
        QShortcut(QKeySequence.ZoomIn, self, self.zoom_in)
        QShortcut(QKeySequence.ZoomOut, self, self.zoom_out)

    def set_fit_to_window(self, fit):
        self._fit = fit
        self.setWidgetResizable(fit)
        self.setSizeAdjustPolicy(
            QScrollArea.AdjustToContents if fit else QScrollArea.AdjustIgnored
        )

    def zoom(self, factor):
        self._scale *= factor
        new_size = self.widget().sizeHint() * self._scale
        self.widget().resize(new_size)

    def zoom_in(self):
        self.zoom(2)

    def zoom_out(self):
        self.zoom(0.5)
