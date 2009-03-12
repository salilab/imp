import unittest
import IMP.test
import IMP.algebra as alg
import math


class Matrix3DTests(IMP.test.TestCase):
    def make_matrix(self, v):
        m = alg.matrix3D(len(v), len(v[0]), len(v[0][0]))
        for z in range(len(v)):
            for y in range(len(v[0])):
                for x in range(len(v[0][0])):
                    m[z,y,x] = v[z][y][x]
        return m

    def assert_matrix_equal(self, m, expected):
        for z in range(len(expected)):
            for y in range(len(expected[0])):
                for x in range(len(expected[0][0])):
                    self.assertEqual(m[z,y,x], expected[z][y][x])

    def assert_matrix_in_tolerance(self, m, expected, tol):
        for z in range(len(expected)):
            for y in range(len(expected[0])):
                for x in range(len(expected[0][0])):
                    self.assertInTolerance(m[z,y,x], expected[z][y][x], tol)

    def test_sizes(self):
        """Check proper creation an resizing"""
        m = alg.matrix3D(3,2,3)
        self.assertEqual(m.get_slices(), 3)
        m.resize(5,7,6)
        self.assertEqual(m.get_slices(), 5)
        self.assertEqual(m.get_rows(), 7)
        self.assertEqual(m.get_columns(), 6)

    def test_access(self):
        """Check matrix3D access"""
        m = alg.matrix3D(3,2,7)
        m[2,1,6]=34.5
        self.assertEqual(m[2,1,6],34.5)

    def test_addition(self):
        """Check Matrix3D addition"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        m2 = self.make_matrix([[[-1,0,1,2],[0,1,2,3]],
                               [[0,1,2,3], [1,2,3,4]]])
        idm1 = id(m1)
        result = m1 + m2
        m1 += m2
        # Inplace operation should not change the Python object identity
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[[-1,1,3,5],[1,3,5,7]],
                                         [[1,3,5,7], [3,5,7,9]]])

    def test_subtraction(self):
        """Check Matrix3D subtraction"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        m2 = self.make_matrix([[[-1,0,1,2],[0,1,2,3]],
                               [[0,1,2,3], [1,2,3,4]]])
        idm1 = id(m1)
        result = m1 - m2
        m1 -= m2
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[[1,1,1,1],[1,1,1,1]],
                                         [[1,1,1,1],[1,1,1,1]]])

    def test_multiplication(self):
        """Check Matrix3D multiplication"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        m2 = self.make_matrix([[[-1,0,1,2],[0,1,2,3]],
                               [[0,1,2,3], [1,2,3,4]]])
        idm1 = id(m1)
        result = m1 * m2
        m1 *= m2
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[[0,0,2,6], [0,2,6,12]],
                                         [[0,2,6,12],[2,6,12,20]]])

    def test_division(self):
        """Check Matrix3D division"""
        m1 = self.make_matrix([[[0,0,2,6], [0,2,6,12]],
                               [[0,2,6,12],[2,6,12,20]]])
        m2 = self.make_matrix([[[-1,1,1,2],[1,1,2,3]],
                               [[1,1,2,3], [1,2,3,4]]])
        idm1 = id(m1)
        result = m1 / m2
        m1 /= m2
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_in_tolerance(r, [[[0,0,2,3],[0,2,3,4]],
                                                [[0,2,3,4],[2,3,4,5]]], 0.001)

    def test_scalar_addition(self):
        """Check Matrix3D scalar addition"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        idm1 = id(m1)
        result = m1 + 3
        m1 += 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[[3,4,5,6],[4,5,6,7]],
                                         [[4,5,6,7],[5,6,7,8]]])

    def test_scalar_subtraction(self):
        """Check Matrix3D scalar subtraction"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        idm1 = id(m1)
        result = m1 - 3
        m1 -= 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[[-3,-2,-1,0],[-2,-1,0,1]],
                                         [[-2,-1,0,1], [-1,0,1,2]]])

    def test_scalar_multiplication(self):
        """Check Matrix3D scalar multiplication"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        idm1 = id(m1)
        result = m1 * 3
        m1 *= 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[[0,3,6,9], [3,6,9,12]],
                                         [[3,6,9,12],[6,9,12,15]]])

    def test_scalar_division(self):
        """Check Matrix3D scalar division"""
        m1 = self.make_matrix([[[0,1,2,3],[1,2,3,4]],
                               [[1,2,3,4],[2,3,4,5]]])
        idm1 = id(m1)
        result = m1 / 3
        m1 /= 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_in_tolerance(r,
                                            [[[0.000,0.333,0.666,1.000],
                                              [0.333,0.666,1.000,1.333]],
                                             [[0.333,0.666,1.000,1.333],
                                              [0.666,1.000,1.333,1.666]]],
                                            0.001)

if __name__ == '__main__':
    unittest.main()
