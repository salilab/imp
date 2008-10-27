import unittest
import IMP.utils
import IMP.test, IMP
import IMP.core

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def setUp(self):
        """Set up distance restraints"""
        IMP.test.TestCase.setUp(self)
    def test_bestn(self):
        """Testing LowestNRestraintSet"""
        m= IMP.Model()
        lrs= IMP.core.LowestNRestraintSet(5)
        m.add_restraint(lrs)
        for i in range(0,10):
            c= IMP.core.ConstantRestraint(i)
            lrs.add_restraint(c)
        self.assertEqual(m.evaluate(False),
                         0+1+2+3+4)

if __name__ == '__main__':
    unittest.main()
