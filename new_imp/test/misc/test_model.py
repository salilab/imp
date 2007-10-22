import unittest
import IMP
import IMP.test

class ModelTests(IMP.test.IMPTestCase):
    def test_state(self):
        """Check get_state"""
        m = IMP.Model()
        self.assertRaises(IndexError, m.get_state, IMP.StateIndex(0));

    def test_restraints(self):
        """Check get_restraint"""
        m = IMP.Model()
        self.assertRaises(IndexError, m.get_restraint, IMP.RestraintIndex(0));

if __name__ == '__main__':
    unittest.main()
