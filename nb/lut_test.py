import cppimport
import unittest

lut = cppimport.imp_from_filepath("lut.cpp")

from lut import Table

class LutTest(unittest.TestCase):
    def test_empty(self):
        table = Table([[]])
        self.assertEqual(table.entries, [])

    def test_single(self):
        table = Table([[0x11, 0x22, 0x33, 0x44]])
        self.assertEqual([hex(e) for e in table.entries],
                         [hex(0x44112233)])

    def test_multi(self):
        table = Table([[0x11, 0x22, 0x33, 0x44],
                       [0x55, 0x66, 0x77, 0x88]])
        self.assertEqual([hex(e) for e in table.entries],
                         [hex(0x44112233),
                          hex(0x88556677)])

if __name__ == '__main__':
    unittest.main()
