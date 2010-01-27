import unittest
import IMP
import IMP.test

class RestraintSetTests(IMP.test.TestCase):
    """Test RestraintSets"""

    def _make_stuff(self):
        m= IMP.Model()
        rs= IMP.RestraintSet(.5)
        r0= IMP.test.ConstRestraint(1)
        rs.add_restraint(r0)
        m.add_restraint(rs)
        r1= IMP.test.ConstRestraint(1)
        rs.add_restraint(r1)
        r2= IMP.test.ConstRestraint(1)
        m.add_restraint(r2)
        return (m, rs, r0, r1, r2)

    def test_weights(self):
        """Test that sets can be weighted"""
        (m, rs, r0, r1, r2)= self._make_stuff()
        self.assertEqual(m.evaluate(False), 2)
        rs.set_weight(1)
        self.assertEqual(m.evaluate(False), 3)

    def test_restraints(self):
        """Check access to RestraintSet's restraints"""
        (m, rs, r0, r1, r2)= self._make_stuff()
        rsrs= rs.get_restraints()
        self.assertEqual(rsrs.count(r0), 1)
        self.assertEqual(rsrs.count(r1), 1)
        self.assertEqual(rsrs.count(r2), 0)

    def test_evaluate(self):
        """Test evaluate() of RestraintSets and their children"""
        (m, rs, r0, r1, r2)= self._make_stuff()
        self.assertEqual(rs.evaluate(False), 1)
        self.assertEqual(r0.evaluate(False), 1)
    def test_evaluate_2(self):
        """Test restraints added multiple times"""
        (m, rs, r0, r1, r2)= self._make_stuff()
        m.add_restraint(r0)
        self.assertEqual(m.evaluate(False), 3)

if __name__ == '__main__':
    unittest.main()
