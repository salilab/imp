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
        """Test that restraints with a zillion terms are squashed"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            l0= IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m)
            l1= IMP.atom.read_pdb(self.get_input_file_name("simple.pdb"), m)
            cpc= IMP.container.CloseBipartitePairContainer(IMP.atom.get_leaves(l0), IMP.atom.get_leaves(l1), 15, 1)
            r= IMP.container.PairsRestraint(IMP.core.HarmonicDistancePairScore(10, 10), cpc)
            rmf= RMF.create_rmf_file(self.get_tmp_file_name("zillion."+suffix))
            IMP.rmf.add_hierarchies(rmf, [l0, l1])
            IMP.rmf.add_restraints(rmf, [r])
            IMP.rmf.set_maximum_number_of_terms(rmf, 10)
            IMP.rmf.save_frame(rmf, 0)
            num_nodes= self._get_num_nodes(rmf.get_root_node())
            print num_nodes
            self.assertEqual(num_nodes,
                             m.get_number_of_particles()+1+1+2*len(IMP.atom.get_leaves(l0)))
if __name__ == '__main__':
    unittest.main()
