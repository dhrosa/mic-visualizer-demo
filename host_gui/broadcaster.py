from threading import Lock
from queue import Queue


class Broadcaster:
    def __init__(self):
        self.sinks_lock = Lock()
        self.sinks = {}
        self.next_sink_id = 0

    def broadcast(self, value):
        with self.sinks_lock:
            sinks = tuple(self.sinks.values())
            for sink in sinks:
                sink.put(value)

    def subscribe(self):
        sink = Queue()
        with self.sinks_lock:
            sink_id = self.next_sink_id
            self.next_sink_id += 1
            self.sinks[sink_id] = sink
        try:
            while True:
                item = sink.get()
                if item is None:
                    break
                yield item
        finally:
            with self.sinks_lock:
                del self.sinks[sink_id]
