import IMP
import IMP.test
import IMP.algebra
import math


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

if __name__ == '__main__':
    IMP.test.main()
