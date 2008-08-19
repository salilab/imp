import unittest
import IMP
import IMP.test

class TestNonbondedRestraint(IMP.test.TestCase):
    """Tests specifically for the NonbondedRestraint. (Most tests for nonbonded
       lists are found in ../states/test_nonbonded_list.py.)"""

    def test_interacting_particles(self):
        """Test NonbondedRestraint::get_interacting_particles() method"""
        rk = IMP.FloatKey("radius")
        m = IMP.Model()
        p = self.create_point_particle(m, 0, 0, 0)
        p.add_attribute(rk, 1.0, False)
        s = IMP.AllNonbondedListScoreState(5.0, rk)
        s.set_particles(m.get_particles())
        m.add_score_state(s)
        o = IMP.test.ConstPairScore(1)
        r = IMP.NonbondedRestraint(o, s)
        m.add_restraint(r)
        self.assertRaises(RuntimeError, r.get_interacting_particles)

if __name__ == '__main__':
    unittest.main()
