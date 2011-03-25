import unittest
import IMP.hdf5
import IMP.test
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _assert_same(self, h0, h1):
        self.assertAlmostEqual(IMP.atom.get_mass(h0),
                               IMP.atom.get_mass(h1), delta=1)
        self.assertEqual(len(IMP.atom.get_leaves(h0)),
                         len(IMP.atom.get_leaves(h1)))
    def test_round_trip(self):
        """Test reading and writing geometry"""
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("geometry.hdf5"), True)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10, 10, 10))
        g= IMP.display.BoundingBoxGeometry(bb)
        IMP.hdf5.write_geometry(g, f)
        gs= IMP.hdf5.read_all_geometries(f, 0)
        self.assertEqual(len(gs), 1)
        gs[0].set_was_used(True)
        cg= gs[0].get_components()
        self.assertEqual(len(cg), 12)

if __name__ == '__main__':
    unittest.main()
