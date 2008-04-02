import unittest
import IMP
import IMP.test
import math

class Vector3DTests(IMP.test.TestCase):
    def test_magnitude(self):
        """Check Vector3D magnitude"""
        v = IMP.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(v.get_squared_magnitude(), 14.0)
        self.assertAlmostEqual(v.get_magnitude(), math.sqrt(14.0), places=1)

    def test_component(self):
        """Check Vector3D components"""
        v = IMP.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(v[0], 1.0)
        self.assertEqual(v[1], 2.0)
        self.assertEqual(v[2], 3.0)
        v[0] = 10.0
        self.assertEqual(v[0], 10.0)

    def test_scalar_product(self):
        """Check Vector3D scalar product"""
        v1 = IMP.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.Vector3D(10.0, 1.0, 2.0)
        self.assertEqual(v1.scalar_product(v2), v2.scalar_product(v1))
        self.assertEqual(v1.scalar_product(v2), v1 * v2)
        self.assertEqual(v1.scalar_product(v2), v2 * v1)
        self.assertEqual(v1.scalar_product(v2), 18.0)

    def test_vector_product(self):
        """Check Vector3D vector product"""
        v1 = IMP.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.Vector3D(10.0, 1.0, 2.0)
        v12 = v1.vector_product(v2)
        v21 = v2.vector_product(v1)
        expected_v12 = (1.0, 28.0, -19.0)
        for i in range(3):
            self.assertEqual(v12[i], -v21[i])
            self.assertEqual(v12[i], expected_v12[i])

    def test_difference(self):
        """Check Vector3D difference"""
        v1 = IMP.Vector3D(1.0, 2.0, 3.0)
        v2 = IMP.Vector3D(10.0, 1.0, 2.0)
        diff = v1 - v2
        expected_diff = (-9.0, 1.0, 1.0)
        for i in range(3):
            self.assertEqual(diff[i], expected_diff[i])

if __name__ == '__main__':
    unittest.main()
