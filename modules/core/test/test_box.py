import IMP
import IMP.test
import IMP.algebra
import IMP.core
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    """Test the symmetry restraint"""

    def test_symmetry(self):
        """Test the box score"""
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(p)
        bbi = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(10, 10, 10),
                                        IMP.algebra.Vector3D(20, 20, 20))
        bbo = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                        IMP.algebra.Vector3D(30, 30, 30))

        d.set_coordinates(IMP.algebra.get_random_vector_in(bbo))
        d.get_coordinates().show()
        d.set_coordinates_are_optimized(True)
        s = IMP.core.BoundingBox3DSingletonScore(IMP.core.Harmonic(0, 1), bbi)
        r = IMP.core.SingletonRestraint(m, s, p)
        sf = IMP.core.RestraintsScoringFunction([r])
        o = IMP.core.ConjugateGradients(m)
        o.set_scoring_function(sf)
        o.optimize(100)
        for i in range(0, 3):
            self.assertGreater(d.get_coordinate(i), 9.9)
            self.assertLess(d.get_coordinate(i), 20.1)
        d.get_coordinates().show()

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test the box score with JAX"""
        import IMP.jax
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(p)
        bbi = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(10, 10, 10),
                                        IMP.algebra.Vector3D(20, 20, 20))
        s = IMP.core.BoundingBox3DSingletonScore(IMP.core.Harmonic(0, 1), bbi)
        r = IMP.core.SingletonRestraint(m, s, p)

        pts = ((11, 12, 13), (1, 2, 3), (30, 40, 50))
        m.set_number_of_sphere_attribute_sets(len(pts))
        imp_scores = []
        for i, pt in enumerate(pts):
            m.set_active_sphere_attribute_set(i)
            if IMP.core.XYZ.get_is_setup(p):
                d = IMP.core.XYZ(p)
            else:
                d = IMP.core.XYZ.setup_particle(p)
            d.set_coordinates(IMP.algebra.Vector3D(pt))
            imp_scores.append(r.evaluate(False))
        jax_score = r._evaluate_jax()
        for i in range(len(pts)):
            self.assertAlmostEqual(jax_score[i], imp_scores[i], delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_periodic(self):
        """Test the box score with JAX with PBC"""
        import IMP.jax
        import jax.numpy as jnp
        space = IMP.jax.PeriodicSpace([4., 4., 4.])
        m = IMP.Model()
        p = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(p)
        d.set_coordinates(IMP.algebra.Vector3D(0., 0., 0.))
        bbi = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(10, 10, 10),
                                        IMP.algebra.Vector3D(20, 20, 20))
        s = IMP.core.BoundingBox3DSingletonScore(IMP.core.Harmonic(0, 1), bbi)
        ji = s._get_jax(m, jnp.array([p.get_index()]), space=space)
        jm = ji.get_jax_model()
        f = jax.jit(ji.score_func)
        self.assertAlmostEqual(f(jm), 6.0, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
