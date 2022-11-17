import unittest
import numpy as np
from numpy.testing import assert_equal as assert_array_equal

from lut import Table


class LutEntryTest(unittest.TestCase):
    def test_empty(self):
        with self.assertRaisesRegex(ValueError, "empty"):
            Table([[]])

    def test_too_few_dimensions(self):
        with self.assertRaises(ValueError, msg="Array must be 2D:"):
            Table(np.zeros((1, 1, 4)))

    def test_too_few_color_channels(self):
        with self.assertRaises(ValueError, msg="Innermost dimension must be 4: 3"):
            Table(np.zeros((1, 3)))

    def test_too_many_color_channels(self):
        with self.assertRaises(ValueError, msg="Innermost dimension must b 4: 5"):
            Table(np.zeros((1, 5)))

    def test_single(self):
        table = Table(
            [
                [0x11, 0x22, 0x33, 0x44],
            ]
        )
        self.assertEqual(
            [hex(e) for e in table.entries],
            [hex(0x44112233)],
        )

    def test_multi(self):
        table = Table(
            [
                [0x11, 0x22, 0x33, 0x44],
                [0x55, 0x66, 0x77, 0x88],
            ]
        )
        self.assertEqual(
            [hex(e) for e in table.entries],
            [
                hex(0x44112233),
                hex(0x88556677),
            ],
        )


class LutMapTest(unittest.TestCase):
    def test_single(self):
        table = Table([[0, 0, 1, 0]])
        source = np.array(
            [
                [10, 12.5],
                [15, 20],
            ]
        )
        dest = np.zeros_like(source, dtype="uint32")
        table.Map(10, 20, source, dest)

        assert_array_equal(
            [
                [1, 1],
                [1, 1],
            ],
            dest,
        )

    def test_multi(self):
        table = Table(
            [
                [0, 0, 1, 0],
                [0, 0, 2, 0],
                [0, 0, 3, 0],
                [0, 0, 4, 0],
            ]
        )
        # Overly small values should map to 1
        # Overly large values should map to 4
        source = np.array(
            [
                [0.9, 1.0, 1.5],
                [1.75, 2.0, 2.1],
            ]
        )
        dest = np.zeros_like(source, dtype="uint32")
        table.Map(1, 2, source, dest)
        assert_array_equal(
            [
                [1, 1, 2],
                [3, 4, 4],
            ],
            dest,
        )


if __name__ == "__main__":
    unittest.main()
