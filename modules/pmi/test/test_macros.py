from __future__ import print_function
import IMP.test
import IMP.pmi.macros

class Tests(IMP.test.TestCase):
    def test_rmf_restraints(self):
        """Test RMFRestraints class"""
        class MockPMIRestraint(object):
            def __init__(self, r):
                self.r = r
            get_restraint = lambda self: self.r
        m = IMP.Model()
        for ur in ([], None):
            rs = IMP.pmi.macros._RMFRestraints(m, ur)
            self.assertFalse(rs)
            self.assertEqual(len(rs), 0)

        r1 = IMP.RestraintSet(m)
        IMP.pmi.tools.add_restraint_to_model(m, r1, add_to_rmf=True)

        rs = IMP.pmi.macros._RMFRestraints(m, [MockPMIRestraint(42),
                                               MockPMIRestraint(99)])
        self.assertTrue(rs)
        self.assertEqual(len(rs), 3)
        self.assertEqual(rs[0].get_restraint(), 42)
        self.assertEqual(rs[1].get_restraint(), 99)
        self.assertEqual(rs[2].get_restraint(), r1)
        self.assertRaises(IndexError, rs.__getitem__, 3)
        self.assertRaises(IndexError, rs.__getitem__, -1)


if __name__ == '__main__':
    IMP.test.main()
