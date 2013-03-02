import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_large(self):
        """Test multi frame files"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            p =IMP.Particle(m)
            h= IMP.atom.Hierarchy.setup_particle(p)
            d= IMP.core.XYZR.setup_particle(p)
            md= IMP.atom.Mass.setup_particle(p, 1)
            d.set_radius(1)
            d.set_x(0)
            d.set_y(0)
            d.set_z(0)
            nf=10
            IMP.base.set_log_level(IMP.base.VERBOSE)
            path=self.get_tmp_file_name("test_mf."+suffix)
            f=RMF.create_rmf_file(path)
            IMP.rmf.add_hierarchy(f, h)
            for i in range(0,nf):
                d.set_x(i)
                IMP.rmf.save_frame(f, i)
            del f
            f= RMF.open_rmf_file_read_only(path)
            [h]= IMP.rmf.create_hierarchies(f, m)
            for i in range(0,nf):
                IMP.rmf.load_frame( f, i)
                d= IMP.core.XYZR(h)
                self.assertEqual(d.get_x(), i)

if __name__ == '__main__':
    unittest.main()
