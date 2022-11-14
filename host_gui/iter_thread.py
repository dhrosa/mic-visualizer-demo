from threading import Thread
from inspect import isgenerator, isgeneratorfunction


class IterThread(Thread):
    def __init__(self, gen):
        super().__init__()
        if isgeneratorfunction(gen):
            gen = gen()
        if not isgenerator(gen):
            raise TypeError("`gen` must be a generator or generator function.")
        self.gen = gen
        self.closing = False
        self.start()

    def __del__(self):
        self.close()

    def close(self):
        self.closing = True
        self.join()

    def run(self):
        while not self.closing:
            next(self.gen)
