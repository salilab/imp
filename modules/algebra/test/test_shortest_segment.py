import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

    def test_sphere_construction(self):
        """Check degenerate shortest segments of segments"""
        V= IMP.algebra.Vector3D
        S= IMP.algebra.Segment3D
        ss= IMP.algebra.get_shortest_segment
        sa= S(V(0,0,0), V(0,0,10))
        sb= S(V(1,0,0), V(1,0,10))
        ssab= ss(sa,sb)
        print ssab
        self.assertAlmostEqual(ssab.get_length(), 1, delta=.1)
        sa= S(V(0,0,10), V(0,0,0))
        sb= S(V(1,0,0), V(1,0,10))
        ssab= ss(sa,sb)
        print ssab
        self.assertAlmostEqual(ssab.get_length(), 1, delta=.1)
        sa= S(V(0,0,-10), V(0,0,0))
        sb= S(V(1,0,0), V(1,0,10))
        ssab= ss(sa,sb)
        print ssab
        self.assertAlmostEqual(ssab.get_length(), 1, delta=.1)
if __name__ == '__main__':
    IMP.test.main()
