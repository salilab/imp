import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_removal(self):
        """Check removal of CenterOfMass decorator"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        xyz1 = IMP.core.XYZ.setup_particle(p1, [0, 0, 0])
        mass1 = IMP.atom.Mass.setup_particle(p1, 1.0)

        p2 = IMP.Particle(m)
        com = IMP.atom.CenterOfMass.setup_particle(p2, [p1])
        # Should be a ScoreState to update COM
        self.assertEqual(len(m.get_score_states()), 1)
        self.assertEqual(len(m.get_particle_indexes()), 2)

        # Remove decorator and ScoreState
        IMP.atom.CenterOfMass.teardown_particle(com)
        self.assertEqual(len(m.get_score_states()), 0)
        m.update()

        # It should now be OK to remove the particle too
        m.remove_particle(p2)
        self.assertEqual(len(m.get_particle_indexes()), 1)
        m.update()

    def test_constraint_accessors(self):
        """Check accessors of CenterOfMass-created constraint"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        xyz1 = IMP.core.XYZ.setup_particle(p1, [0, 0, 0])
        mass1 = IMP.atom.Mass.setup_particle(p1, 1.0)

        p2 = IMP.Particle(m)
        com = IMP.atom.CenterOfMass.setup_particle(p2, [p1])
        ss = m.score_states[0].get_derived_object()
        self.assertIsInstance(ss, IMP.core.SingletonConstraint)
        bm = ss.get_before_modifier().get_derived_object()
        self.assertIsInstance(bm, IMP.core.CentroidOfRefined)
        self.assertFalse(bm.get_is_weight_null())
        weight = bm.get_weight()
        self.assertEqual(weight.get_string(), "mass")
        refiner = bm.get_refiner()
        self.assertIsInstance(refiner, IMP.Refiner)


if __name__ == '__main__':
    IMP.test.main()
