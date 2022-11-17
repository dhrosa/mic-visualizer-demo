from funcy import memoize

_events = {}


@memoize
def _startup_time():
    import os

    return os.stat("/proc/self").st_mtime_ns / 1e9


def record_first_event(key):
    if key in _events:
        return
    from time import time
    from si_prefix import si_format

    t = time()
    _events[key] = t
    dt = t - _startup_time()
    print(f"Time to first {key} event: {dt} s / {si_format(dt)}s", flush=True)
