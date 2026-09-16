import IMP
import IMP.test
try:
    import jax
    import IMP._jax_util
    import jax.numpy as jnp
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space(self):
        """Test JAX FreeSpace"""
        space = IMP._jax_util.FreeSpace
        drs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        dists = jnp.array([3.74165, 8.77496])
        self.assertTrue(jnp.allclose(space.distance(drs), dists))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space(self):
        """Test JAX PeriodicSpace"""
        space = IMP._jax_util.PeriodicSpace([10., 20., 30.])
        # For vector components less than half the box size, distances
        # should be the same as FreeSpace
        drs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        dists = jnp.array([3.74165, 8.77496])
        self.assertTrue(jnp.allclose(space.distance(drs), dists))

        # Components should wrap over half the box size
        drs = jnp.array([[9., 2., 3.], [24., 5., 6.]])
        dists = jnp.array([3.74165, 8.77496])
        self.assertTrue(jnp.allclose(space.distance(drs), dists))


if __name__ == '__main__':
    IMP.test.main()


if __name__ == '__main__':
    IMP.test.main()
