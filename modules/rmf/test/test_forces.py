import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def _show(self, g):
        for i in range(0, g.get_number_of_children()):
            print i, g.get_child_name(i), g.get_child_is_group(i)
    def test_forces(self):
        """Test saving forces"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            print "reading pdb"
            name=self.get_tmp_file_name("test_round_trip."+suffix)
            h= IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m,
                                 IMP.atom.NonAlternativePDBSelector())
            h.get_is_valid(True)
            IMP.base.set_log_level(IMP.base.SILENT)
            IMP.atom.add_bonds(h)
            r = IMP.core.ExcludedVolumeRestraint(IMP.atom.get_leaves(h), 1)
            r.create_scoring_function().evaluate(False)

            f= RMF.create_rmf_file(name)
            IMP.rmf.add_hierarchy(f, h)
            IMP.rmf.set_save_forces(f, True)
            IMP.rmf.save_frame(f, 0)
            ff = RMF.ForceFactory(f)
            for a in IMP.atom.get_leaves(h):
                nh = IMP.rmf.get_node_from_association(f, a.get_particle())
                fd = ff.get(nh)
                force = fd.get_force()
                deriv = IMP.core.XYZ(a).get_derivatives()
                for i in range(0, 3):
                    self.assertEqual(force[i], -deriv[i])

if __name__ == '__main__':
    unittest.main()
