import IMP
import IMP.test
import IMP.algebra
import math
import StringIO

class Tests(IMP.test.TestCase):

    def test_triangle_construction(self):
        """Check that triangles are constructed correctly"""
        s = IMP.algebra.Triangle3D(IMP.algebra.Vector3D(0.0, 0.0, 0.0),
                                   IMP.algebra.Vector3D(0.0, 0.0, 5.0),
                                   IMP.algebra.Vector3D(0.0, 5.0, 0.0),
                                   )
        lens = s.get_edge_lengths()
        self.assertEqual(lens[0], 5)
        self.assertEqual(lens[1], 5)
        self.assertAlmostEqual(lens[2], 7.07, 1)
        self.assertEqual([x for x in s.get_point(0)], [0, 0, 0])
        self.assertEqual([x for x in s.get_point(1)], [0, 0, 5])
        self.assertEqual([x for x in s.get_point(2)], [0, 5, 0])
        sio = StringIO.StringIO()
        s.show(sio)

    def test_get_are_colinear(self):
        """Check get_are_colinear()"""
        col = [IMP.algebra.Vector3D(0., 0., 0.),
               IMP.algebra.Vector3D(0., 10., 0.),
               IMP.algebra.Vector3D(0., 20., 0.)]
        self.assertEqual(IMP.algebra.get_are_colinear(*col), True)
        not_col = [IMP.algebra.Vector3D(0., 0., 0.),
                   IMP.algebra.Vector3D(0., 10., 0.),
                   IMP.algebra.Vector3D(0., 0., 10.)]
        self.assertEqual(IMP.algebra.get_are_colinear(*not_col), False)

    def test_get_largest_triangle(self):
        """Check get_largest_triangle()"""
        t = [IMP.algebra.Vector3D(0., 0., 0.),
             IMP.algebra.Vector3D(0., 10., 0.),
             IMP.algebra.Vector3D(10., 10., 0.),
             IMP.algebra.Vector3D(30., 0., 0.),
             IMP.algebra.Vector3D(0., 0., 10.)]
        s = IMP.algebra.get_largest_triangle(t)
        self.assertEqual([x for x in s.get_point(0)], [0, 10, 0])
        self.assertEqual([x for x in s.get_point(1)], [30, 0, 0])
        self.assertEqual([x for x in s.get_point(2)], [0, 0, 10])

    def test_triangle_trans(self):
        """Check triangle transformations"""
        V = IMP.algebra.Vector3D
        t1 = IMP.algebra.Triangle3D(V(0.0, 0.0, 0.0),
                                    V(5.0, 0.0, 0.0),
                                    V(0.0, 5.0, 0.0))
        t2 = IMP.algebra.Triangle3D(V(0.4, 0.0, 0.0),
                                    V(0.4, 5.0, 0.0),
                                    V(0.4, 0.0, 5.0))
        t = IMP.algebra.get_transformation_from_first_triangle_to_second(t1, t2)
        v = t.get_translation()
        self.assertLess(IMP.algebra.get_distance(v, V(0.4,0,0)), 1e-4)
        r = t.get_rotation().get_quaternion()
        self.assertAlmostEqual(r[0], 0.5, delta=1e-5)
        self.assertAlmostEqual(r[1], 0.5, delta=1e-5)
        self.assertAlmostEqual(r[2], 0.5, delta=1e-5)
        self.assertAlmostEqual(r[3], 0.5, delta=1e-5)

if __name__ == '__main__':
    IMP.test.main()
