from __future__ import print_function, division
import IMP
import IMP.algebra
import IMP.core
import IMP.pmi
import IMP.pmi.restraints
import IMP.test


class Tests(IMP.test.TestCase):

    def test_functions(self):

        class DistanceRestraint(IMP.pmi.restraints.RestraintBase):
            def __init__(self, p1, p2, d, k):
                m = p1.get_model()
                super(DistanceRestraint, self).__init__(m)
                f = IMP.core.Harmonic(d, k)
                s = IMP.core.DistancePairScore(f)
                r = IMP.core.PairRestraint(self.m, s, (p1, p2))
                self.rs.add_restraint(r)

        m = IMP.Model()
        p1 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p1)
        p2 = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p2)
        r = DistanceRestraint(p1, p2, 0., 1.)
        self.assertAlmostEqual(r.evaluate(), 0.0, delta=1e-6)
        output = r.get_output()
        self.assertEqual(output["DistanceRestraint_Score"], str(0.0))
        self.assertEqual(output["_TotalScore"], str(0.0))
        self.assertIsInstance(r.get_restraint_set(), IMP.RestraintSet)

        IMP.core.XYZ(p2).set_coordinates(IMP.algebra.Vector3D(0, 0, 10))
        r.set_weight(10.)
        r.set_label("Test")
        self.assertAlmostEqual(r.evaluate(), 500, delta=1e-6)
        output = r.get_output()
        self.assertEqual(output["DistanceRestraint_Score_Test"], str(500.0))
        self.assertEqual(output["_TotalScore"], str(500.0))
        self.assertIsInstance(r.get_restraint_set(), IMP.RestraintSet)


if __name__ == '__main__':
    IMP.test.main()
