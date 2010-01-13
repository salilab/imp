import unittest
import IMP.algebra
import IMP.test
import math

class Vector4DTests(IMP.test.TestCase):
    def test_magnitude(self):
        """Check Vector4D magnitude"""
        v = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v.get_squared_magnitude(), 30.0)
        self.assertAlmostEqual(v.get_magnitude(), math.sqrt(30.0), places=1)

    def test_component(self):
        """Check Vector4D components"""
        v = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        self.assertEqual(v[0], 1.0)
        self.assertEqual(v[1], 2.0)
        self.assertEqual(v[2], 3.0)
        self.assertEqual(v[3], 4.0)
        v[0] = 10.0
        self.assertEqual(v[0], 10.0)

    def test_scalar_product(self):
        """Check Vector4D scalar product"""
        v1 = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        v2 = IMP.algebra.Vector4D(10.0, 1.0, 2.0, 3.0)
        self.assertInTolerance(v1.scalar_product(v2), v2.scalar_product(v1),
                               .1)
        self.assertInTolerance(v1.scalar_product(v2), v1 * v2,
                               .1)
        self.assertInTolerance(v1.scalar_product(v2), v2 * v1, .1)
        self.assertInTolerance(v1.scalar_product(v2), 30.0, .1)

    def test_difference(self):
        """Check Vector4D difference"""
        v1 = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        v2 = IMP.algebra.Vector4D(10.0, 1.0, 2.0, 3.0)
        diff = v1 - v2
        v1 -= v2
        expected_diff = IMP.algebra.Vector4D(-9.0, 1.0, 1.0, 1.0)
        self.assertInTolerance((diff-expected_diff).get_magnitude(),
                               0, .1)
        self.assertInTolerance((v1-expected_diff).get_magnitude(),
                               0, .1)

    def test_addition(self):
        """Check Vector4D addition"""
        v1 = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        idv1 = id(v1)
        cppobj = str(v1.this)
        v2 = IMP.algebra.Vector4D(10.0, 1.0, 2.0, 3.0)
        sum = v1 + v2
        v1 += v2
        # Inplace addition should not change the Python object identity:
        self.assertEqual(id(v1), idv1)
        # The underlying C++ object pointer should be unchanged too:
        self.assertEqual(str(v1.this), cppobj)
        expected_sum = IMP.algebra.Vector4D(11.0, 3.0, 5.0, 7.0)
        self.assertInTolerance((sum-expected_sum).get_magnitude(),
                               0, .1)
        self.assertInTolerance((v1-expected_sum).get_magnitude(),
                               0, .1)

    def test_scalar_multiplication(self):
        """Check Vector4D multiplication by a scalar"""
        v1 = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 4.0)
        idv1 = id(v1)
        s1 = 3.0
        prod = v1 * s1
        v1 *= s1
        # Inplace multiplication should not change the Python object identity:
        self.assertEqual(id(v1), idv1)
        expected_prod = (3.0, 6.0, 9.0, 12.0)
        for i in range(3):
            self.assertInTolerance(prod[i], expected_prod[i],
                                   .1)
            self.assertInTolerance(v1[i], expected_prod[i],
                                   .1)

    def test_scalar_division(self):
        """Check Vector4D division by a scalar"""
        v1 = IMP.algebra.Vector4D(3.0, 6.0, 9.0, 27.0)
        idv1 = id(v1)
        s1 = 3.0
        prod = v1 / s1
        v1 /= s1
        # Inplace division should not change the Python object identity:
        self.assertEqual(id(v1), idv1)
        expected_prod = (1.0, 2.0, 3.0, 9.0)
        for i in range(3):
            self.assertEqual(prod[i], expected_prod[i])
            self.assertEqual(v1[i], expected_prod[i])

    def test_distance(self):
        """Distance should not work for 4D vectors"""
        v1 = IMP.algebra.Vector4D(3.0, 6.0, 9.0, 0.0)
        v2 = IMP.algebra.Vector4D(1.0, 2.0, 3.0, 10.0)
        self.assertRaises(TypeError, IMP.algebra.squared_distance, v1, v2)

    def test_rotation_from_vector4d(self):
        """Check creation of a rotation from a 4D vector"""
        r= IMP.algebra.Rotation3D(1,0,0,0)
        v1= r.get_quaternion()
        v2= IMP.algebra.Vector4D(1.0, 2.0, 3.0, 10.0)
        r2= IMP.algebra.rotation_from_vector4d(v2)

if __name__ == '__main__':
    unittest.main()
