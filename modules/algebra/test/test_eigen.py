import IMP.test
import IMP.algebra
import StringIO
import os
import math


class Tests(IMP.test.TestCase):

    def test_magnitude(self):
        """Check pass eigen types works"""
        inval = [[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12]]
        out = IMP.algebra._pass_matrix(inval)
        self.assertEqual(inval, out)

if __name__ == '__main__':
    IMP.test.main()
