import unittest
from circular_buffer import CircularBuffer
from numpy.testing import assert_array_equal


def assert_buffers(data, older, newer):
    a, b = data.buffers
    assert_array_equal(a, older)
    assert_array_equal(b, newer)


class CircularBufferTest(unittest.TestCase):
    def test_empty(self):
        data = CircularBuffer(3, 2)
        assert_buffers(
            data,
            older=[
                [0, 0, 0],
                [0, 0, 0],
            ],
            newer=[
                [],
                [],
            ],
        )

    def test_single_append(self):
        data = CircularBuffer(3, 2)
        data.append([1, 2])

        assert_buffers(
            data,
            older=[
                [0, 0],
                [0, 0],
            ],
            newer=[
                [1],
                [2],
            ],
        )

    def test_double_append(self):
        data = CircularBuffer(3, 2)
        data.append([1, 2])
        data.append([3, 4])

        assert_buffers(
            data,
            older=[
                [0],
                [0],
            ],
            newer=[
                [1, 3],
                [2, 4],
            ],
        )

    def test_complete_cycle(self):
        data = CircularBuffer(3, 2)
        data.append([1, 2])
        data.append([3, 4])
        data.append([5, 6])

        assert_buffers(
            data,
            older=[
                [1, 3, 5],
                [2, 4, 6],
            ],
            newer=[
                [],
                [],
            ],
        )

    def test_rollover(self):
        data = CircularBuffer(3, 2)
        data.append([1, 2])
        data.append([3, 4])
        data.append([5, 6])
        data.append([7, 8])

        assert_buffers(
            data,
            older=[
                [3, 5],
                [4, 6],
            ],
            newer=[
                [7],
                [8],
            ],
        )


if __name__ == "__main__":
    unittest.main()
