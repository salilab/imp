import unittest
import IMP
import IMP.test
import IMP.algebra
import math

class BoundingBoxTests(IMP.test.TestCase):

    def test_bounding_box_construction(self):
        """Check that the bounding box is constructed correctly"""
        points = []
        bound1 = IMP.algebra.Vector3D(0.0,10.0,0.0)
        bound2 = IMP.algebra.Vector3D(10.0,0.0,10.0)
        points.append(bound1)
        points.append(bound2)
        small_box= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1,1,1),
                                            IMP.algebra.Vector3D(9,9,9))
        for i in range(0,20):
            v= IMP.algebra.random_vector_in_box(small_box)
            points.append(v)
        bb=IMP.algebra.BoundingBox3D(points)
        self.assertAlmostEqual(IMP.algebra.distance(bb.get_corner(0),IMP.algebra.Vector3D(0,0,0)),0.001,places=1)
        self.assertAlmostEqual(IMP.algebra.distance(bb.get_corner(1),IMP.algebra.Vector3D(10,10,10)),0.001,places=1)
if __name__ == '__main__':
    unittest.main()
