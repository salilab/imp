import IMP
import IMP.test
import IMP.core
try:
    import jax
    import IMP.jax
except ImportError:
    jax = None


fkey = IMP.FloatKey("my float key")


def make_score(key, multi=False):
    m = IMP.Model()
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 6., 7.), 8.0))
    p1.add_attribute(fkey, 42.0)
    s = IMP.core.AttributeSingletonScore(IMP.core.Linear(0.0, 10.0), key)
    r = IMP.core.SingletonRestraint(m, s, p1)

    if multi:
        m.set_number_of_sphere_attribute_sets(2)
        m.set_number_of_attribute_sets(fkey, 2)
        m.set_active_sphere_attribute_set(1)
        m.set_active_attribute_set(fkey, 1)
        IMP.core.XYZR.setup_particle(
            p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(15., 16., 17.), 9.0))
        p1.add_attribute(fkey, 99.0)
        m.set_active_sphere_attribute_set(0)
        m.set_active_attribute_set(fkey, 0)
    return m, p1, s, r


class Tests(IMP.test.TestCase):

    def test_score(self):
        """Test AttributeSingletonScore value"""
        # xyz and radius keys are handled specially in IMP
        m, p1, s, r = make_score(IMP.core.XYZ.get_xyz_keys()[0])
        self.assertAlmostEqual(r.evaluate(False), 50.0, delta=1e-4)

        m, p1, s, r = make_score(IMP.core.XYZR.get_radius_key())
        self.assertAlmostEqual(r.evaluate(False), 80.0, delta=1e-4)

        m, p1, s, r = make_score(fkey)
        self.assertAlmostEqual(r.evaluate(False), 420.0, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of AttributeSingletonScore"""
        m, p1, s, r = make_score(IMP.core.XYZ.get_xyz_keys()[0])
        imp_score = r.evaluate(False)
        jax_score = r._evaluate_jax()
        self.assertAlmostEqual(imp_score, 50.0, delta=1e-4)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-4)

        m, p1, s, r = make_score(IMP.core.XYZR.get_radius_key())
        imp_score = r.evaluate(False)
        jax_score = r._evaluate_jax()
        self.assertAlmostEqual(imp_score, 80.0, delta=1e-4)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-4)

        m, p1, s, r = make_score(fkey)
        imp_score = r.evaluate(False)
        jax_score = r._evaluate_jax()
        self.assertAlmostEqual(imp_score, 420.0, delta=1e-4)
        self.assertAlmostEqual(imp_score, jax_score, delta=1e-4)

        # No support yet for JAX scores on rigid body local coordinates
        m, p1, s, r = make_score(IMP.FloatKey("local_x"))
        self.assertRaises(NotImplementedError, r._get_jax,
                          space=IMP.jax.FreeSpace)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multi(self):
        """Test JAX AttributeSingletonScore with multiple attrsets"""
        m, p1, s, r = make_score(IMP.core.XYZ.get_xyz_keys()[0], multi=True)
        imp_score1 = r.evaluate(False)
        m.set_active_sphere_attribute_set(1)
        imp_score2 = r.evaluate(False)
        jax_score = r._evaluate_jax()
        self.assertAlmostEqual(imp_score1, 50.0, delta=1e-4)
        self.assertAlmostEqual(imp_score1, jax_score[0], delta=1e-4)
        self.assertAlmostEqual(imp_score2, 150.0, delta=1e-4)
        self.assertAlmostEqual(imp_score2, jax_score[1], delta=1e-4)

        m, p1, s, r = make_score(fkey, multi=True)
        imp_score1 = r.evaluate(False)
        m.set_active_attribute_set(fkey, 1)
        imp_score2 = r.evaluate(False)
        jax_score = r._evaluate_jax()
        self.assertAlmostEqual(imp_score1, 420.0, delta=1e-4)
        self.assertAlmostEqual(imp_score1, jax_score[0], delta=1e-4)
        self.assertAlmostEqual(imp_score2, 990.0, delta=1e-4)
        self.assertAlmostEqual(imp_score2, jax_score[1], delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
