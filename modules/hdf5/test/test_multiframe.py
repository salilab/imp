import unittest
import IMP.hdf5
import IMP.test
from IMP.algebra import *

class GenericTest(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    """Test the python code"""
    def test_large(self):
        """Test multi frame files"""
        m= IMP.Model()
        p =IMP.Particle(m)
        h= IMP.atom.Hierarchy.setup_particle(p)
        d= IMP.core.XYZR.setup_particle(p)
        d.set_radius(1)
        d.set_x(0)
        d.set_y(0)
        d.set_z(0)
        nf=10
        IMP.set_log_level(IMP.PROGRESS)
        f=IMP.hdf5.RootHandle(self.get_tmp_file_name("test_mf.rh"), True)
        IMP.hdf5.write_hierarchy(h, f)
        for i in range(0,nf):
            d.set_x(i)
            IMP.hdf5.save_conformation(h, f, i)
        del f
        f= IMP.hdf5.RootHandle(self.get_tmp_file_name("test_mf.rh"), False)
        [h]= IMP.hdf5.read_all_hierarchies(f, m)
        for i in range(0,nf):
            IMP.hdf5.load_conformation( f, h, i)
            d= IMP.core.XYZR(h)
            self.assertEqual(d.get_x(), i)

if __name__ == '__main__':
    unittest.main()
