import IMP
import IMP.test
import IMP.algebra
import IMP.core
import IMP.example
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def test_pair_score(self):
        """Test example PairScore"""
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        # Test both implementations: C++ and Python
        for typ in (IMP.example.ExamplePairScore,
                    IMP.example.PythonExamplePairScore):
            p = typ(2.0, 10.0)
            da = IMP.DerivativeAccumulator()
            self.assertAlmostEqual(p.evaluate_index(m, [p1, p2], da),
                                   51.08, delta=0.01)
            # Note that we can't test derivatives because they haven't been
            # initialized
            self.assertIn("PairScore", str(p))
            self.assertIn("PairScore", repr(p))
            self.assertIn("example", p.get_version_info().get_module())
            self.assertEqual(len(p.get_inputs(m, [p1,p2])), 2)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation"""
        import jax.numpy as jnp
        import IMP.jax
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        p = IMP.example.ExamplePairScore(2.0, 10.0)

        ji = p._get_jax(m, jnp.array([[p1, p2]]),
                        space=IMP.jax.FreeSpace)
        X = ji.get_jax_model()
        f = jax.jit(ji.score_func)
        self.assertAlmostEqual(f(X), 51.08, delta=0.01)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multi(self):
        """Test JAX implementation with multiple attrsets"""
        import jax.numpy as jnp
        import IMP.jax
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        p = IMP.example.ExamplePairScore(2.0, 10.0)

        m.set_number_of_sphere_attribute_sets(2)
        imp_score1 = p.evaluate_index(m, (p1, p2), None)
        m.set_active_sphere_attribute_set(1)
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(10,20,30))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(40,50,60))
        imp_score2 = p.evaluate_index(m, (p1, p2), None)

        ji = p._get_jax(m, jnp.array([[p1, p2]]),
                        space=IMP.jax.FreeSpace)
        jm = ji.get_jax_model()
        f = jax.jit(ji.score_func)
        jax_score = f(jm)
        self.assertEqual(jax_score.shape, (2, 1))
        self.assertAlmostEqual(jax_score[0], imp_score1, delta=0.01)
        self.assertAlmostEqual(jax_score[1], imp_score2, delta=0.01)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_periodic(self):
        """Test JAX implementation with PBC"""
        import jax.numpy as jnp
        import IMP.jax
        space = IMP.jax.PeriodicSpace([4., 4., 4.])
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        p = IMP.example.ExamplePairScore(2.0, 10.0)

        ji = p._get_jax(m, jnp.array([[p1, p2]]), space=space)
        X = ji.get_jax_model()
        f = jax.jit(ji.score_func)
        self.assertAlmostEqual(f(X), 0.35898393, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
