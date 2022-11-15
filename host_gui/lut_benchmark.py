import cppimport
import logging
import sys
import numpy as np
import timeit
from argparse import ArgumentParser
from si_prefix import si_format


root_logger = logging.getLogger()
root_logger.setLevel(logging.DEBUG)

handler = logging.StreamHandler(sys.stdout)
handler.setLevel(logging.DEBUG)
formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
handler.setFormatter(formatter)
root_logger.addHandler(handler)

cppimport.settings["force_rebuild"] = True

from lut import Table # noqa: 402

if __name__ == "__main__":
    parser = ArgumentParser(description="Benchmark for C++ LUT implementation")
    parser.add_argument(
        "--lut_size", type=int, default=100, help="Number of entries in lookup table."
    )
    parser.add_argument("--width", type=int, default=3840, help="Width of destination.")
    parser.add_argument(
        "--height", type=int, default=2160, help="Height of destination."
    )
    parser.add_argument(
        "--benchmark_duration",
        type=int,
        default=3,
        help="Target duration of benchmark in seconds.",
    )

    args = parser.parse_args()
    print(args)

    rng = np.random.default_rng()
    table = Table(
        rng.integers(
            low=0, high=255, endpoint=True, size=(args.lut_size, 4), dtype="uint8"
        )
    )

    vmin = 10
    vmax = 20

    shape = (args.height, args.width)
    source = rng.uniform(low=vmin * 0.95, high=vmax * 1.05, size=shape)
    dest = np.zeros(shape, dtype="uint32")

    def lut_map():
        table.Map(vmin, vmax, source, dest)

    timer = timeit.Timer(stmt="lut_map()", globals=globals())
    auto_loops, auto_duration = timer.autorange()
    print(f"Autorange suggests {auto_loops} loops over {si_format(auto_duration)}s")

    loops = int(auto_loops * args.benchmark_duration / auto_duration)

    duration = timer.timeit(loops)
    print(f"{loops} loops over {si_format(duration)}s")
    print(f"{si_format(loops/duration)}loops per second")
    print(f"{si_format(duration / loops)}s per loop")
