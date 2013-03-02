import unittest
import IMP.rmf
import IMP.atom
import IMP.test
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def _get_num_nodes(self, n):
        return 1+sum([self._get_num_nodes(c) for c in n.get_children()])
    def test_round_trip(self):
        """Test that exceptions are transformed"""
        m= IMP.Model()
        rmf= RMF.create_rmf_file(self.get_tmp_file_name("zillion.rmf"))
        h= IMP.atom.Fragment.setup_particle(IMP.Particle(m))
        IMP.core.XYZR.setup_particle(h)
        IMP.atom.Mass.setup_particle(h, 1)
        IMP.rmf.add_hierarchies(rmf, [h])
        IMP.rmf.save_frame(rmf, 0)
        self.assertRaises(IMP.IOException, IMP.rmf.load_frame, rmf, 6)
    def test_in_python(self):
        """Test that RMF exceptions can be caught in python"""
        self.assertRaises(IOError, RMF.test_throw_exception)
if __name__ == '__main__':
    unittest.main()
