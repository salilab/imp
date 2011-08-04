import unittest
import IMP.rmf
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
        f= IMP.rmf.create_rmf_file(self.get_tmp_file_name("geometry.hdf5"))
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10, 10, 10))
        g= IMP.display.BoundingBoxGeometry(bb)
        IMP.rmf.add_geometry(f, g)
        gs= IMP.rmf.create_geometries(f, 0)
        self.assertEqual(len(gs), 1)
        gs[0].set_was_used(True)
        cg= gs[0].get_components()
        self.assertEqual(len(cg), 12)
    def test_3(self):
        """Testing surface geometry"""
        if not hasattr(IMP, 'cgal'):
            self.skipTest("IMP.cgal module disabled")
        g=IMP.algebra.DenseDoubleGrid3D(1, IMP.algebra.BoundingBox3D((-10, -10, -10),
                                                                     (10,10,10)))
        for i in g.get_all_indexes():
            c= g.get_center(i)
            m= c.get_magnitude()
            g[i]=100-m
        #for i in g.get_all_indexes():
        #    print i, g.get_center(i), g[i]
        gg= IMP.display.IsosurfaceGeometry(g, 95.0)
        gg.set_name("isosurface")
        rmf= IMP.rmf.create_rmf_file(self.get_tmp_file_name("iso.rmf"))
        IMP.rmf.add_geometry(rmf, gg)
        del rmf
        rmf= IMP.rmf.open_rmf_file(self.get_tmp_file_name("iso.rmf"))
        gs= IMP.rmf.create_geometries(rmf,0)
        w=IMP.display.PymolWriter(self.get_tmp_file_name("iso.pym"))
        w.add_geometry(gg)
        gs[0].set_name("after")
        w.add_geometry(gs[0])

if __name__ == '__main__':
    unittest.main()
