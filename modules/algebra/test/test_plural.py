import unittest
import IMP.algebra
import IMP.test

class PluralTests(IMP.test.TestCase):

    def test_plural(self):
        """Test all algebra objects have plural version"""
        return self.assertPlural(IMP.algebra, ['FixedXYZ', 'FixedZYZ', 'LinearFit', 'Matrix2D', 'Matrix3D', 'MultiArray3D', 'NearestNeighbor2D', 'NearestNeighbor3D', 'NearestNeighbor4D', 'ParabolicFit', 'BoundingBox2D', 'BoundingBox3D', 'BoundingBox4D'])

    def test_show(self):
        """Test all algebra objects have show"""
        return self.assertShow(IMP.algebra, ['AxisAnglePair', 'Matrix3D', 'MultiArray3D', 'NearestNeighbor2D', 'NearestNeighbor3D', 'NearestNeighbor4D'])

if __name__ == '__main__':
    unittest.main()
