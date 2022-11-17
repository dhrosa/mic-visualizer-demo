#! /usr/bin/env python3

from asyncio import create_subprocess_exec, run, subprocess
import re
from argparse import ArgumentParser
from si_prefix import si_format
from time import time

PIPE = subprocess.PIPE
pattern = re.compile(r"first ImageViewer.paintEvent event: (\d+\.?\d+) ")


async def trial(timer):
    process = await create_subprocess_exec("./main.py", stdin=PIPE, stdout=PIPE)
    async for line in process.stdout:
        line = line.decode().strip()
        if not line:
            continue
        print(line, end=None)
        if match := pattern.search(line):
            process.terminate()
            await process.wait()
            timer.report(float(match[1]))


class TrialTimer:
    def __init__(self):
        self._durations = []

    def report(self, duration):
        self._durations.append(duration)

    def __call__(self):
        return sum(self._durations)


def main():
    parser = ArgumentParser(description="Benchmark for GUI startup latency.")
    parser.add_argument(
        "--benchmark_duration",
        type=float,
        default=10,
        help="Target duration of benchmark in seconds.",
    )
    args = parser.parse_args()
    print(f"{args=}")

    from timeit import Timer

    trial_timer = TrialTimer()
    timer = Timer(
        stmt="run(trial(trial_timer))", globals=globals() | locals(), timer=trial_timer
    )
    auto_loops, auto_duration = timer.autorange()
    print(f"Autorange suggests {auto_loops} loops over {si_format(auto_duration)}s")

    loops = int(auto_loops * args.benchmark_duration / auto_duration)
    duration = timer.timeit(loops)

    print(f"{loops} loops over {si_format(duration)}s")
    print(f"{si_format(loops/duration)}loops per second")
    print(f"{si_format(duration / loops)}s per loop")

    from pandas import Series, to_timedelta

    print(
        Series(trial_timer._durations[1:], name="ms").mul(1000).describe().to_frame().T
    )


if __name__ == "__main__":
    main()
