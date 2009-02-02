import unittest
import IMP
import IMP.test
import IMP.search
import IMP.core
import IMP.modeller


class GenericTest(IMP.test.TestCase):
    """Test AttributeMap1"""

    def _test_myrestraint(self):
        """Checking AttributeMap1ScoreState"""
        m = IMP.Model()
        d= IMP.modeller.read_pdb('modules/search/test/states/single_protein.pdb', m)
        atoms= IMP.core.get_by_type(d, IMP.core.MolecularHierarchyDecorator.RESIDUE)
        pc= IMP.core.ListSingletonContainer(atoms)
        ss= IMP.search.IntMapScoreState(pc, IMP.search.IntMapKey(IMP.core.ResidueDecorator.get_index_key()))
        m.add_score_state(ss)
        m.evaluate(False)
        ps= ss.get_particles(IMP.search.IntMapValue(10))
        for p in ps:
            print p.get_index().get_index()
        self.assert_(len(ps) == 1)
        ps= ss.get_particles(IMP.search.IntMapValue(10),
                             IMP.search.IntMapValue(20))
        for p in ps:
            print p.get_index().get_index()
        self.assert_(len(ps) == 10)


if __name__ == '__main__':
    unittest.main()
