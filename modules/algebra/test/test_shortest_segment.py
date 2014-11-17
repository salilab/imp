import IMP
import IMP.test
import IMP.algebra
import math


class Tests(IMP.test.TestCase):

    def test_sphere_construction(self):
        """Check degenerate shortest segments of segments"""
        V = IMP.algebra.Vector3D
        S = IMP.algebra.Segment3D
        ss = IMP.algebra.get_shortest_segment
        sa = S(V(0, 0, 0), V(0, 0, 10))
        sb = S(V(1, 0, 0), V(1, 0, 10))
        ssab = ss(sa, sb)
        print ssab
        self.assertAlmostEqual(ssab.get_length(), 1, delta=.1)
        self.assertAlmostEqual(IMP.algebra.get_distance(sa, sb), 1, delta=.1)
        sa = S(V(0, 0, 10), V(0, 0, 0))
        sb = S(V(1, 0, 0), V(1, 0, 10))
        ssab = ss(sa, sb)
        print ssab
        self.assertAlmostEqual(ssab.get_length(), 1, delta=.1)
        self.assertAlmostEqual(IMP.algebra.get_distance(sa, sb), 1, delta=.1)
        sa = S(V(0, 0, -10), V(0, 0, 0))
        sb = S(V(1, 0, 0), V(1, 0, 10))
        ssab = ss(sa, sb)
        print ssab
        self.assertAlmostEqual(ssab.get_length(), 1, delta=.1)
        self.assertAlmostEqual(IMP.algebra.get_distance(sa, sb), 1, delta=.1)

    def test_too_short(self):
        """Check shortest segments with too short segments"""
        V = IMP.algebra.Vector3D
        S = IMP.algebra.Segment3D
        ss = IMP.algebra.get_shortest_segment
        sa = S(V(0, 0, 0), V(0, 0, 0))
        sb = S(V(1, 0, 0), V(1, 0, 10))
        ssab = ss(sa, sb)
        self.assertLess(IMP.algebra.get_distance(ssab.get_point(0), V(0,0,0)),
                        1e-3)
        self.assertLess(IMP.algebra.get_distance(ssab.get_point(1), V(1,0,0)),
                        1e-3)
        ssba = ss(sb, sa)
        self.assertLess(IMP.algebra.get_distance(ssba.get_point(0), V(1,0,0)),
                        1e-3)
        self.assertLess(IMP.algebra.get_distance(ssba.get_point(1), V(0,0,0)),
                        1e-3)
        ssaa = ss(sa, S(V(1, 0, 0), V(1, 0, 0)))
        self.assertLess(IMP.algebra.get_distance(ssaa.get_point(0), V(0,0,0)),
                        1e-3)
        self.assertLess(IMP.algebra.get_distance(ssaa.get_point(1), V(1,0,0)),
                        1e-3)

    def test_non_parallel(self):
        """Check shortest segments with non parallel segments"""
        V = IMP.algebra.Vector3D
        S = IMP.algebra.Segment3D
        ss = IMP.algebra.get_shortest_segment
        sa = S(V(0, 0, 0), V(0, 0, 10))
        sb = S(V(3, 0, 4), V(10, 0, 4))
        ssab = ss(sa, sb)
        self.assertLess(IMP.algebra.get_distance(ssab.get_point(0), V(0,0,4)),
                        1e-3)
        self.assertLess(IMP.algebra.get_distance(ssab.get_point(1), V(3,0,4)),
                        1e-3)

    def test_parallel_no_overlap(self):
        """Check shortest segments parallel non overlapping segments"""
        V = IMP.algebra.Vector3D
        S = IMP.algebra.Segment3D
        ss = IMP.algebra.get_shortest_segment
        sa = S(V(0, 0, 0), V(0, 0, 10))
        sb = S(V(0, 0, 20), V(0, 0, 30))
        ssab = ss(sa, sb)
        self.assertAlmostEqual(ssab.get_length(), 10, delta=.1)

if __name__ == '__main__':
    IMP.test.main()
