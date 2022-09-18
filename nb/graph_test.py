import sys

from audio_widget import Context

from PySide6.QtWidgets import QApplication


def main():
    import audio_widget
    app = QApplication(sys.argv)
    context = Context(app)
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
