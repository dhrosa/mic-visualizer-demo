#! /usr/bin/env python3

from matplotlib import colormaps
import numpy as np

# Sequential colormaps from
# https://matplotlib.org/stable/tutorials/colors/colormaps.html#classes-of-colormaps
_cmap_names = [
    "viridis",
    "plasma",
    "inferno",
    "magma",
    "cividis",
    "Greys",
    "Purples",
    "Blues",
    "Greens",
    "Oranges",
    "Reds",
    "YlOrBr",
    "YlOrRd",
    "OrRd",
    "PuRd",
    "RdPu",
    "BuPu",
    "GnBu",
    "PuBu",
    "YlGnBu",
    "PuBuGn",
    "BuGn",
    "YlGn",
    "binary",
    "gist_yarg",
    "gist_gray",
    "gray",
    "bone",
    "pink",
    "spring",
    "summer",
    "autumn",
    "winter",
    "cool",
    "Wistia",
    "hot",
    "afmhot",
    "gist_heat",
    "copper",
]

def cmap_lut_entries(cmap_name, n):
    return colormaps.get_cmap(cmap_name)(np.linspace(0, 1, n, endpoint=True), alpha=1, bytes=True)

def array_entry_str(color):
    r, g, b, a = color
    digits = "".join(f'{d:02X}' for d in [a, r, g, b])
    return f"0x{digits}ULL"

def array_str(colors):
    entry_strs = ', '.join(map(array_entry_str, colors))
    return f'{{ {entry_strs}  }}'

def colormap_entry_str(cmap_name, n):
    colors = cmap_lut_entries(cmap_name, n)
    return f'ColorMap{{ .name = "{cmap_name}", .entries = {array_str(colors)} }},'

def colormaps_str(cmap_names, n):
    colormap_entry_strings = "\n  ".join(colormap_entry_str(c, n) for c in cmap_names)
    return f'#include "colormaps.h"\n\nconstexpr ColorMap kColorMaps[] = {{\n{colormap_entry_strings}\n}};'


if __name__ == '__main__':
    print(colormaps_str(_cmap_names, 256))
