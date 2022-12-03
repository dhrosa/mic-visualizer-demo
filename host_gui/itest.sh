#!/ bin / bash

find..- type f | grep - v../ build / |
    entr - r sh - c 'cmake --build . && ctest --output-on-failure'
