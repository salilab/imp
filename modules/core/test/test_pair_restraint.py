import IMP
import IMP.test
import IMP.core
try:
    import jax
except ImportError:
    jax = None


def make_score(multi=False):
    m = IMP.Model()
    s = IMP.core.SoftSpherePairScore(1)
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(4., 0., 0.), 1.0))
    p2 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 1., 0.), 2.0))
    p3 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p3, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(6., 1., 0.), 2.0))

    if multi:
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        IMP.core.XYZR.setup_particle(
            p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(8., 0., 0.), 1.0))
        IMP.core.XYZR.setup_particle(
            p2, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(15., 6., 7.), 2.0))
        IMP.core.XYZR.setup_particle(
            p3, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(16., 6., 7.), 2.0))

    return m, p1, p2, p3, s


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_fused(self):
        """Test that JAX fuses multiple PairRestraint with same score"""
        m, p1, p2, p3, s = make_score()
        r1 = IMP.core.PairRestraint(m, s, (p1, p2))
        r1.set_weight(0.2)
        r2 = IMP.core.PairRestraint(m, s, (p2, p3))
        r2.set_weight(0.5)
        # r1 and r2 both use the same score object `s`; RestraintSet should
        # score them together (using get_jax_multiple())

        # r3 uses a different score object (should be scored separately)
        s2 = IMP.core.HarmonicDistancePairScore(1.0, 1.0)
        r3 = IMP.core.PairRestraint(m, s2, (p1, p3))
        r3.set_weight(0.4)
        rs = IMP.RestraintSet(m)
        rs.restraints.extend((r1, r2, r3))
        imp_score = rs.evaluate(False)
        jax_score = rs._evaluate_jax()
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-4)
        # Score should match that from restraints scored individually
        self.assertAlmostEqual(
            jax_score,
            sum(r._evaluate_jax() for r in (r1, r2, r3)), delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multi_attr(self):
        """Test single JAX PairRestraint with multi attrsets"""
        m, p1, p2, p3, s = make_score()
        r1 = IMP.core.PairRestraint(m, s, (p1, p2))
        rs = IMP.RestraintSet(m)
        rs.restraints.append(r1)
        imp_score1 = r1.evaluate(False)

        # Second attribute set
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        IMP.core.XYZR.setup_particle(
            m, p1, IMP.algebra.Sphere3D((4., 5., 6.), 1.0))
        IMP.core.XYZR.setup_particle(
            m, p2, IMP.algebra.Sphere3D((5., 5., 7.), 1.0))
        IMP.core.XYZR.setup_particle(
            m, p3, IMP.algebra.Sphere3D((1., 2., 3.), 1.0))
        imp_score2 = r1.evaluate(False)

        jax_score = r1._evaluate_jax()
        self.assertEqual(jax_score.shape, (2,))
        self.assertAlmostEqual(jax_score[0], imp_score1, delta=1e-4)
        self.assertAlmostEqual(jax_score[1], imp_score2, delta=1e-4)

        # Same score should be returned via RestraintSet
        jax_score_rs = rs._evaluate_jax()
        self.assertEqual(jax_score_rs.shape, (2,))
        self.assertAlmostEqual(jax_score_rs[0], imp_score1, delta=1e-4)
        self.assertAlmostEqual(jax_score_rs[1], imp_score2, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multi_attr_multi_rsrs(self):
        """Test multiple JAX PairRestraints with multi attrsets"""
        m, p1, p2, p3, s = make_score()
        r1 = IMP.core.PairRestraint(m, s, (p1, p2))
        r1.set_weight(0.2)
        r2 = IMP.core.PairRestraint(m, s, (p2, p3))
        r2.set_weight(0.5)
        rs = IMP.RestraintSet(m)
        rs.restraints.extend((r1, r2))

        imp_score1 = rs.evaluate(False)

        # Second attribute set
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        IMP.core.XYZR.setup_particle(
            m, p1, IMP.algebra.Sphere3D((4., 5., 6.), 1.0))
        IMP.core.XYZR.setup_particle(
            m, p2, IMP.algebra.Sphere3D((5., 5., 7.), 1.0))
        IMP.core.XYZR.setup_particle(
            m, p3, IMP.algebra.Sphere3D((1., 2., 3.), 1.0))
        imp_score2 = rs.evaluate(False)

        jax_score = rs._evaluate_jax()
        self.assertEqual(jax_score.shape, (2,))
        self.assertAlmostEqual(jax_score[0], imp_score1, delta=1e-4)
        self.assertAlmostEqual(jax_score[1], imp_score2, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
