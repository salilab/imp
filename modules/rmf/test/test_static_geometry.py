from __future__ import print_function
import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *


class Tests(IMP.test.TestCase):

    def test_round_trip(self):
        """Test reading and writing static geometry"""
        for suffix in IMP.rmf.suffixes:
            nm = self.get_tmp_file_name("static_geometry" + suffix)
            print(nm)
            f = RMF.create_rmf_file(nm)
            bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                           IMP.algebra.Vector3D(10, 10, 10))
            g = IMP.display.BoundingBoxGeometry(bb)
            IMP.rmf.add_static_geometries(f, [g])
            # saving empty frames is not happy in rmf
            IMP.rmf.save_frame(f, str(0))
            IMP.rmf.save_frame(f, str(1))
            del f
            f = RMF.open_rmf_file_read_only(nm)
            gs = IMP.rmf.create_geometries(f)
            self.assertEqual(len(gs), 1)
            gs[0].set_was_used(True)
            cg = gs[0].get_components()
            self.assertEqual(len(cg), 12)
            IMP.rmf.load_frame(f, RMF.FrameID(1))

    def test_complex_geometry(self):
        """Test reading and writing complex static geometry"""
        m = IMP.Model()
        ds = IMP.core.create_xyzr_particles(m, 20, .1)
        sc = IMP.container.ListSingletonContainer(m, ds)
        cpc = IMP.container.ConnectingPairContainer(sc, .1)
        m.update()
        pg = IMP.core.EdgePairsGeometry(cpc)

        for suffix in IMP.rmf.suffixes:
            nm = self.get_tmp_file_name("static_geometry" + suffix)
            f = RMF.create_rmf_file(nm)
            IMP.rmf.add_static_geometries(f, [pg])
            IMP.rmf.save_frame(f, str(0))
            del f
            f = RMF.open_rmf_file_read_only(nm)
            gs = IMP.rmf.create_geometries(f)
            # On reread, complex geometry is broken up into components
            self.assertEqual(len(gs), 19)
            for g in gs:
                g.set_was_used(True)
                cg = g.get_components()
                self.assertEqual(len(cg), 1)

if __name__ == '__main__':
    IMP.test.main()
