import IMP
import IMP.test
import IMP.atom
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-pickle) of implicit RestraintSet scoring function"""
        m = IMP.Model()
        rs = IMP.RestraintSet(m, .5, "RS")
        r0 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r0)
        mc = IMP.atom.MolecularDynamics(m)
        mc.set_scoring_function(rs)

        dump = pickle.dumps(mc)
        newmc = pickle.loads(dump)
        self.assertAlmostEqual(newmc.get_scoring_function().evaluate(False),
                               0.5, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
