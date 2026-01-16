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
        import jax.numpy as jnp
        m = IMP.Model()
        p1 = m.add_particle("p1")
        p2 = m.add_particle("p2")
        d1 = IMP.core.XYZ.setup_particle(m, p1, IMP.algebra.Vector3D(1,2,3))
        d2 = IMP.core.XYZ.setup_particle(m, p2, IMP.algebra.Vector3D(4,5,6))
        p = IMP.example.ExamplePairScore(2.0, 10.0)

        ji = p._get_jax()
        X = ji.get_model_state(m)
        f = jax.jit(ji.score_func)
        self.assertAlmostEqual(f(X, jnp.array([[p1, p2]])), 51.08, delta=0.01)


if __name__ == '__main__':
    IMP.test.main()
