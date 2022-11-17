#!/usr/bin/env python3

import sys

from context import Context

from PySide6.QtCore import QThreadPool
from PySide6.QtWidgets import QApplication

# Qt's event loops prevents Python's default signal handler from being able to
# execute. So instead we hookup the default signal handler which immediately
# exits the process.
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)


def main():
    # The default thread pool name has parentheses inside, which py-spy's thread
    # name parsing doesn't handle correctly.
    QThreadPool.globalInstance().setObjectName("Default QThreadPool")
    app = QApplication(sys.argv)
    Context(app)
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
