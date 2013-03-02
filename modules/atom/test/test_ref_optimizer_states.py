import IMP
import IMP.test
import IMP.atom

class Tests(IMP.test.TestCase):
    """Test refcounting of OptimizerStates"""

    def test_simple(self):
        """Check reference counting of OptimizerStates"""
        ps = []
        m= IMP.Model()
        refcnt = IMP.test.RefCountChecker(self)
        s = IMP.atom.VelocityScalingOptimizerState(ps, 300., 10)
        o = IMP.atom.MolecularDynamics(m)
        o.add_optimizer_state(s)
        refcnt.assert_number(3)
        # Optimizer should hold a ref to state, so nothing should be freed
        # until it is
        del s
        refcnt.assert_number(3)
        del o
        refcnt.assert_number(0)

    def test_delete_optimizer_constructor(self):
        """Constructed Python states should survive optimizer deletion"""
        m=IMP.Model()
        refcnt = IMP.test.RefCountChecker(self)
        o = IMP.atom.MolecularDynamics(m)
        ps = []
        s = IMP.atom.VelocityScalingOptimizerState(ps, 300., 10)
        o.add_optimizer_state(s)
        self.assertEqual(s.get_ref_count(), 2)
        refcnt.assert_number(3)
        # New state s should not go away until we free the Python reference
        del o
        refcnt.assert_number(1)
        self.assertEqual(s.get_ref_count(), 1)
        del s
        refcnt.assert_number(0)

    def test_delete_optimizer_accessor(self):
        "OptimizerStates from vector accessors should survive Optimizer del."
        m= IMP.Model()
        refcnt = IMP.test.RefCountChecker(self)
        o = IMP.atom.MolecularDynamics(m)
        ps = []
        s = IMP.atom.VelocityScalingOptimizerState(ps, 300., 10)
        o.add_optimizer_state(s)
        del s
        # Now create new Python OptimizerState s from a C++ vector accessor
        # These accessors call specific methods in the SWIG wrapper which
        # are modified by typemaps in our interface.
        s = o.get_optimizer_states()[0]
        # Python reference s plus C++ reference from o
        self.assertEqual(s.get_ref_count(), 2)
        del o
        # Now only the Python reference s should survive
        self.assertEqual(s.get_ref_count(), 1)
        refcnt.assert_number(1)
        del s
        refcnt.assert_number(0)


if __name__ == '__main__':
    IMP.test.main()
