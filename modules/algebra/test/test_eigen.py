import IMP.test
import IMP.algebra
import StringIO
import os
import math


class Tests(IMP.test.TestCase):

    def test_matrix(self):
        """Check pass eigen matrix types works"""
        inval = [[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12]]
        out = IMP.algebra._pass_matrix_xf(inval)
        self.assertEqual(inval, out)

        out = IMP.algebra._pass_matrix_xd(inval)
        self.assertEqual(inval, out)

    def test_array(self):
        """Check pass eigen array types works"""
        inval = [[1, 2, 3], [4, 5, 6], [7, 8, 9], [10, 11, 12]]
        out = IMP.algebra._pass_array_xd(inval)
        self.assertEqual(inval, out)

    def test_vector(self):
        """Check pass eigen vector types works"""
        inval = [1, 2, 3]

        out = IMP.algebra._pass_vector_xd(inval)
        self.assertEqual(inval, out)

if __name__ == '__main__':
    IMP.test.main()
