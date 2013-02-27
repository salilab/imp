#!/usr/bin/env python
import unittest
import RMF
import shutil

class GenericTest(unittest.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_perturbed(self):
        """Test example where fill was bad"""
        for suffix in RMF.suffixes:
            nm= RMF._get_temporary_file_path("bad_geom."+suffix)
            rmf= RMF.create_rmf_file(nm)
            rmf.set_current_frame(RMF.ALL_FRAMES)
            colored_factory= RMF.ColoredFactory(rmf)
            ball_factory = RMF.BallFactory(rmf)
            cylinder_factory = RMF.CylinderFactory(rmf)

            dg= rmf.get_root_node().add_child("dynamic geometry", RMF.GEOMETRY)
            bn= dg.add_child("ball", RMF.GEOMETRY)
            cn= dg.add_child("cylinder", RMF.GEOMETRY)

            cd= colored_factory.get(bn)
            cd.set_rgb_color([.5,.75,.95])
            sc= rmf.get_category("shape")
            rk= rmf.get_float_key(sc, "rgb color red")
            print "stuff"
            print rmf.get_float_keys(sc)
            print rk
            print rmf.get_float_key(sc, "rgb color red")
            self.assert_(rk != RMF.FloatKey())
            self.assert_(bn.get_has_value(rk))
            self.assertFalse(cn.get_has_value(rk))

        #cd= cylinder_factory.get(cn, 0)
            self.assert_(bn.get_has_value(rk))
            self.assertFalse(cn.get_has_value(rk))
        #self.assertFalse(sn.get_has_value(rk))

            sg= rmf.get_root_node().add_child("static geometry", RMF.GEOMETRY)
            bns= sg.add_child("ball", RMF.GEOMETRY)
            self.assert_(bn.get_has_value(rk))
            self.assertFalse(cn.get_has_value(rk))
            self.assertFalse(bns.get_has_value(rk))

            bd= ball_factory.get(bns)
            self.assert_(bn.get_has_value(rk))
            self.assertFalse(cn.get_has_value(rk))
            self.assertFalse(bns.get_has_value(rk))
            radk=rmf.get_float_key(sc, "radius")
            bns.set_value(radk, 3)
        #bd.set_radius(3)
            self.assert_(bn.get_has_value(rk))
            self.assertFalse(cn.get_has_value(rk))
            self.assertFalse(bns.get_has_value(rk))


if __name__ == '__main__':
    unittest.main()
