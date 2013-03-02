import IMP
import IMP.test
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

    def test_triangle_construction(self):
        """Check that triangles constructed correctly"""
        s = IMP.algebra.Triangle3D(IMP.algebra.Vector3D(0.0,0.0,0.0),
                                  IMP.algebra.Vector3D(0.0,0.0,5.0),
                                   IMP.algebra.Vector3D(0.0,5.0,0.0),
                                   )
        lens = s.get_edge_lengths()
        self.assertEqual(lens[0],5)
        self.assertEqual(lens[1],5)
        self.assertAlmostEqual(lens[2],7.07,1)
        print lens


if __name__ == '__main__':
    IMP.test.main()
