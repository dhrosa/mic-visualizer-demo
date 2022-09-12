import sys

from audio_widget import MainWindow

from PySide6.QtWidgets import QApplication


def main():
    import audio_widget
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())

if __name__ == '__main__':
    main()
