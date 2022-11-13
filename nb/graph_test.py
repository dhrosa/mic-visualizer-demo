import sys

from context import Context

from PySide6.QtWidgets import QApplication


def main():
    app = QApplication(sys.argv)
    context = Context(app)
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
