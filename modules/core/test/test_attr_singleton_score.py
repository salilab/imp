import IMP
import IMP.test
import IMP.core
try:
    import jax
except ImportError:
    jax = None


fkey = IMP.FloatKey("my float key")


def make_score(key):
    m = IMP.Model()
    p1 = IMP.Particle(m)
    IMP.core.XYZR.setup_particle(
        p1, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5., 6., 7.), 8.0))
    p1.add_attribute(fkey, 42.0)
    s = IMP.core.AttributeSingletonScore(IMP.core.Linear(0.0, 10.0), key)
    r = IMP.core.SingletonRestraint(m, s, p1)
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
        self.assertRaises(NotImplementedError, r._get_jax)


if __name__ == '__main__':
    IMP.test.main()
