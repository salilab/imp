import IMP.test
import IMP.core
try:
    import jax
    import jax.numpy as jnp
    import jax.random
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):
    def test_ball_mover_accessors(self):
        """Test BallMover accessor methods"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        mv = IMP.core.BallMover(m, p1, 1.0)
        self.assertAlmostEqual(mv.get_radius(), 1.0, delta=1e-5)
        self.assertEqual(frozenset(mv.get_keys()),
                         frozenset(IMP.core.XYZ.get_xyz_keys()))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of BallMover"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        d1 = IMP.core.XYZ.setup_particle(m, p1)
        p2 = IMP.Particle(m)
        d = IMP.core.XYZ.setup_particle(m, p2)
        mv = IMP.core.BallMover(m, (p1, p2), 1.0)
        X = {'xyz': jnp.array(m.get_spheres_numpy()[0])}
        j = jax.jit(mv._get_jax())
        k = jax.random.key(42)
        newX, ratio = j(k, X)
        self.assertEqual(newX['xyz'].shape, (2, 3))
        self.assertAlmostEqual(ratio, 1.0, delta=1e-5)
        # Both particles should be moved in the same fashion
        self.assertTrue(jnp.allclose(newX['xyz'][0], newX['xyz'][1]))
        # Particles should not be further than radius from origin
        self.assertLessEqual(jnp.linalg.norm(newX['xyz'][0]), 1.0)


if __name__ == '__main__':
    IMP.test.main()
