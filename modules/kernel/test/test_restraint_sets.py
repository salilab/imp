from __future__ import print_function
import IMP
import IMP.core
import IMP.test


class Tests(IMP.test.TestCase):

    """Test RestraintSets"""

    def _make_stuff(self):
        m = IMP.Model("restraint sets")
        rs = IMP.RestraintSet(m, .5, "RS.5")
        r0 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r0)
        r1 = IMP._ConstRestraint(m, [], 1)
        rs.add_restraint(r1)
        r2 = IMP._ConstRestraint(m, [], 1)
        return (m, rs, r0, r1, r2)

    def test_printing(self):
        """Test that sets can be printed"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        sf = IMP.core.RestraintsScoringFunction([rs, r2])
        self.assertEqual(sf.evaluate(False), 2)
        print(rs)

    def test_restraints(self):
        """Check access to RestraintSet's restraints"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        rsrs = rs.get_restraints()
        self.assertEqual(rsrs.count(r0), 1)
        self.assertEqual(rsrs.count(r1), 1)
        self.assertEqual(rsrs.count(r2), 0)

    def test_restraints_weights(self):
        """Check weights on restraints"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        self.assertEqual(rs.evaluate(False), 1)
        self.assertEqual(r0.evaluate(False), 1)
        r0.set_weight(0)
        self.assertEqual(r0.evaluate(False), 0)
        self.assertEqual(rs.evaluate(False), .5)

    def test_evaluate(self):
        """Test evaluate() of RestraintSets and their children"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        self.assertEqual(rs.evaluate(False), 1)
        self.assertEqual(r0.evaluate(False), 1)

    def test_evaluate_2(self):
        """Test restraints added multiple times"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        sf = IMP.core.RestraintsScoringFunction([rs, r2, r0])
        self.assertEqual(sf.evaluate(False), 3)

    def test_removed(self):
        """Test that restraints are usable after set is destroyed"""
        (m, rs, r0, r1, r2) = self._make_stuff()
        print("print removed", r1.evaluate(False))
        del rs
        print("destroyed", r1.evaluate(False))

if __name__ == '__main__':
    IMP.test.main()
