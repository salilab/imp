import unittest
import IMP.test
import IMP.algebra

class Matrix2DTests(IMP.test.TestCase):
    def make_matrix(self, v):
        m = IMP.algebra.Matrix2D(len(v), len(v[0]))
        for row in range(len(v)):
            for col in range(len(v[0])):
                m[row,col] = v[row][col]
        return m

    def assert_matrix_equal(self, m, expected):
        for row in range(len(expected)):
            for col in range(len(expected[0])):
                self.assertEqual(m[row,col], expected[row][col])

    def assert_matrix_in_tolerance(self, m, expected, tol):
        for row in range(len(expected)):
            for col in range(len(expected[0])):
                self.assertInTolerance(m[row,col], expected[row][col], tol)

    def test_sizes(self):
        """Check proper creation and resizing"""
        m = IMP.algebra.Matrix2D(3,2)
        self.assertEqual(m.get_columns(), 2)
        m.resize(5,7)
        self.assertEqual(m.get_rows(), 5)
        self.assertEqual(m.get_columns(), 7)

    def test_access(self):
        """Check Matrix2D access"""
        m = IMP.algebra.Matrix2D(3,2)
        m[2,1]=34.5
        self.assertEqual(m[2,1],34.5)
        m.set_start(0,-2)
        m.set_start(1,-5)
        m[-1,-4]=34.5
        self.assertEqual(m[-1,-4],34.5)

    def test_addition(self):
        """Check Matrix2D addition"""
        m1 = self.make_matrix([[1,2], [3,4]])
        m2 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 + m2
        m1 += m2
        # Inplace operation should not change the Python object identity
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[2,4], [6,8]])

    def test_subtraction(self):
        """Check Matrix2D subtraction"""
        m1 = self.make_matrix([[1,2], [3,4]])
        m2 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 - m2
        m1 -= m2
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[0,0], [0,0]])

    def test_multiplication(self):
        """Check Matrix2D multiplication"""
        m1 = self.make_matrix([[1,2], [3,4]])
        m2 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 * m2
        m1 *= m2
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[1,4], [9,16]])

    def test_division(self):
        """Check Matrix2D division"""
        m1 = self.make_matrix([[1,2], [3,4]])
        m2 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 / m2
        m1 /= m2
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[1,1], [1,1]])

    def test_scalar_addition(self):
        """Check Matrix2D scalar addition"""
        m1 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 + 3
        m1 += 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[4,5], [6,7]])

    def test_scalar_subtraction(self):
        """Check Matrix2D scalar subtraction"""
        m1 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 - 3
        m1 -= 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[-2,-1], [0,1]])

    def test_scalar_multiplication(self):
        """Check Matrix2D scalar multiplication"""
        m1 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 * 3
        m1 *= 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_equal(r, [[3,6], [9,12]])

    def test_scalar_division(self):
        """Check Matrix2D scalar division"""
        m1 = self.make_matrix([[1,2], [3,4]])
        idm1 = id(m1)
        result = m1 / 3
        m1 /= 3
        self.assertEqual(id(m1), idm1)
        for r in (result, m1):
            self.assert_matrix_in_tolerance(r, [[0.333,0.666], [1.0,1.333]],
                                            0.001)

if __name__ == '__main__':
    unittest.main()
