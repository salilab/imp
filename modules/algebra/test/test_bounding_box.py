import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

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
            v= IMP.algebra.get_random_vector_in(small_box)
            points.append(v)
        bb=IMP.algebra.BoundingBox3D(points)
        self.assertAlmostEqual(IMP.algebra.get_distance(bb.get_corner(0),IMP.algebra.Vector3D(0,0,0)),0.001,places=1)
        self.assertAlmostEqual(IMP.algebra.get_distance(bb.get_corner(1),IMP.algebra.Vector3D(10,10,10)),0.001,places=1)

    def test_bounding_box_union(self):
        """Check the union function"""
        b1= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1,1,1),
                                      IMP.algebra.Vector3D(9,9,9))
        b2= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(-5,-5,-5),
                                      IMP.algebra.Vector3D(3,3,3))
        bup= b1+b2
        bu=IMP.algebra.get_union(b1,b2)
        self.assertAlmostEqual(IMP.algebra.get_distance(bu.get_corner(0),
                                                        IMP.algebra.Vector3D(-5,-5,-5)),0.001,places=1)
        self.assertAlmostEqual(IMP.algebra.get_distance(bu.get_corner(1),
                                                        IMP.algebra.Vector3D(9,9,9)),0.001,places=1)
if __name__ == '__main__':
    IMP.test.main()
