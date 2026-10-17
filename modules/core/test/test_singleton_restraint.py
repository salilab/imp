import IMP
import IMP.test
import IMP.core
import pickle
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def make_system(self):
        m = IMP.Model()
        p1 = m.add_particle("p1")
        IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1., 2., 3.))
        ss = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), IMP.algebra.Vector3D(0., 0., 0.))
        r = IMP.core.SingletonRestraint(m, ss, p1)
        return m, p1, ss, r

    def test_object_info(self):
        """Test object get_type_name() and get_version_info()"""
        m, p1, ss, r = self.make_system()
        self.assertEqual(r.get_type_name(), "SingletonRestraint")
        self.assertEqual(r.get_version_info().get_module(), "IMP::core")

    def test_pickle(self):
        """Test (un-)pickle of SingletonRestraint"""
        m, p1, ss, r = self.make_system()
        r.set_name("foo")
        self.assertAlmostEqual(r.evaluate(False), 7.0, delta=1e-4)
        dump = pickle.dumps(r)
        newr = pickle.loads(dump)
        self.assertAlmostEqual(newr.evaluate(False), 7.0, delta=1e-4)
        self.assertEqual(newr.get_name(), "foo")

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of SingletonRestraint via polymorphic pointer"""
        m, p1, ss, r = self.make_system()
        r.set_name("foo")
        sf = IMP.core.RestraintsScoringFunction([r])
        self.assertAlmostEqual(sf.evaluate(False), 7.0, delta=1e-4)
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        self.assertAlmostEqual(newsf.evaluate(False), 7.0, delta=1e-4)
        newr, = newsf.restraints
        self.assertEqual(newr.get_name(), "foo")

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_fused_multiple(self):
        """Test that JAX fuses multiple SingletonRestraints"""
        m, p1, ss, r1 = self.make_system()
        r1.set_weight(0.2)
        p2 = m.add_particle("p2")
        IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4., 5., 6.))
        r2 = IMP.core.SingletonRestraint(m, ss, p2)
        r2.set_weight(0.4)
        # r1 and r2 both use the same score object `ss`; RestraintSet should
        # score them together (using get_jax_multiple())

        p3 = m.add_particle("p3")
        IMP.core.XYZ.setup_particle(m, p3, IMP.algebra.Vector3D(7., 8., 9.))
        # r3 uses a different score object (should be scored separately)
        ss2 = IMP.core.DistanceToSingletonScore(
                IMP.core.Linear(0, 1), IMP.algebra.Vector3D(0., 0., 0.))
        r3 = IMP.core.SingletonRestraint(m, ss2, p3)
        r2.set_weight(0.7)

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
        """Test single JAX SingletonRestraint with multi attrsets"""
        m, p1, ss, r1 = self.make_system()
        rs = IMP.RestraintSet(m)
        rs.restraints.append(r1)
        imp_score1 = r1.evaluate(False)

        # Second attribute set
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        IMP.core.XYZ.setup_particle(m, p1, (4., 5., 6.))
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
        """Test multiple JAX SingletonRestraints with multi attrsets"""
        m, p1, ss, r1 = self.make_system()
        r1.set_weight(0.2)
        p2 = m.add_particle("p2")
        IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4., 5., 6.))
        r2 = IMP.core.SingletonRestraint(m, ss, p2)
        r2.set_weight(0.4)
        rs = IMP.RestraintSet(m)
        rs.restraints.extend((r1, r2))

        imp_score1 = rs.evaluate(False)

        # Second attribute set
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        IMP.core.XYZ.setup_particle(m, p1, (7., 8., 9.))
        IMP.core.XYZ.setup_particle(m, p2, (10., 11., 12.))
        imp_score2 = rs.evaluate(False)

        jax_score = rs._evaluate_jax()
        self.assertEqual(jax_score.shape, (2,))
        self.assertAlmostEqual(jax_score[0], imp_score1, delta=1e-4)
        self.assertAlmostEqual(jax_score[1], imp_score2, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
