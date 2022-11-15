import sys

from context import Context

from PySide6.QtCore import QThreadPool
from PySide6.QtWidgets import QApplication


def main():
    # The default thread pool name has parentheses inside, which py-spy's thread
    # name parsing doesn't handle correctly.
    QThreadPool.globalInstance().setObjectName("Default QThreadPool")
    app = QApplication(sys.argv)
    Context(app)
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
