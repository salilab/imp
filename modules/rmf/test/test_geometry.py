import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _assert_same(self, h0, h1):
        self.assertAlmostEqual(IMP.atom.get_mass(h0),
                               IMP.atom.get_mass(h1), delta=1)
        self.assertEqual(len(IMP.atom.get_leaves(h0)),
                         len(IMP.atom.get_leaves(h1)))
    def test_round_trip(self):
        """Test reading and writing geometry"""
        nm=self.get_tmp_file_name("geometry.rmf")
        f= RMF.create_rmf_file(nm)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(10, 10, 10))
        g= IMP.display.BoundingBoxGeometry(bb)
        IMP.rmf.add_geometry(f, g)
        del f
        f= RMF.open_rmf_file_read_only(nm)
        gs= IMP.rmf.create_geometries(f)
        self.assertEqual(len(gs), 1)
        gs[0].set_was_used(True)
        cg= gs[0].get_components()
        self.assertEqual(len(cg), 12)
    def _do_test_round_trip(self, g):
        gg= IMP.display.create_geometry(g)
        nm=self.get_tmp_file_name("round_trip_g.rmf")
        rmf= RMF.create_rmf_file(nm)
        IMP.rmf.add_geometries(rmf.get_root_node(), [gg])
        IMP.rmf.save_frame(rmf, 0)
        del rmf
        rmf=RMF.open_rmf_file_read_only(nm)
        ggb= IMP.rmf.create_geometries(rmf)
        IMP.rmf.load_frame(rmf, 0)
        ggbt= gg.get_from(gg)
        self.assertEqual(type(ggbt.get_geometry()), type(g))
    def test_all_geometry(self):
        """Test reading and writing each type of geometry"""
        for g in [IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,1,2),3),
                  IMP.algebra.Segment3D(IMP.algebra.Vector3D(0,1,2),
                                        IMP.algebra.Vector3D(3,4,5)),
                  IMP.algebra.Cylinder3D(IMP.algebra.Segment3D(IMP.algebra.Vector3D(0,1,2),
                                                               IMP.algebra.Vector3D(3,4,5)),6)]:
            self._do_test_round_trip(g)
    def test_3(self):
        """Testing surface geometry"""
        if not hasattr(IMP, 'cgal'):
            self.skipTest("IMP.cgal module disabled")
        #self.skipTest("surface geometry is disabled")
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
        rmf= RMF.create_rmf_file(self.get_tmp_file_name("iso.rmf"))
        IMP.rmf.add_geometry(rmf, gg)
        del rmf
        rmf= RMF.open_rmf_file(self.get_tmp_file_name("iso.rmf"))
        gs= IMP.rmf.create_geometries(rmf)
        w=IMP.display.PymolWriter(self.get_tmp_file_name("iso.pym"))
        w.add_geometry(gg)
        gs[0].set_name("after")
        w.add_geometry(gs[0])

if __name__ == '__main__':
    unittest.main()
