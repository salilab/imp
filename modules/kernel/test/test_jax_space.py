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
    def test_free_space_distance(self):
        """Test JAX FreeSpace distance"""
        space = IMP._jax_util.FreeSpace
        drs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        dists = jnp.array([3.74165, 8.77496])
        self.assertTrue(jnp.allclose(space.distance(drs), dists))
        # Should work for a single vector too
        dr = jnp.array([1., 2., 3.])
        self.assertAlmostEqual(space.distance(dr), dists[0], delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space_shift(self):
        """Test JAX FreeSpace shift"""
        space = IMP._jax_util.FreeSpace
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        drs = jnp.array([[10., 20., 30.], [40., 50., 60.]])
        new_rs = space.shift(rs, drs)
        expected_rs = jnp.array([[11., 22., 33.], [44., 55., 66.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space_shift_indexes(self):
        """Test JAX FreeSpace shift_indexes"""
        space = IMP._jax_util.FreeSpace
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        indexes = jnp.array([0])
        drs = jnp.array([[10., 20., 30.]])
        new_rs = space.shift_indexes(rs, indexes, drs)
        expected_rs = jnp.array([[11., 22., 33.], [4., 5., 6.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_distance(self):
        """Test JAX PeriodicSpace distance"""
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

        # Should work for a single vector too
        dr = jnp.array([9., 2., 3.])
        self.assertAlmostEqual(space.distance(dr), dists[0], delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_shift(self):
        """Test JAX PeriodicSpace shift"""
        space = IMP._jax_util.PeriodicSpace([10., 10., 10.])
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        drs = jnp.array([[7., 8., 9.], [10., 11., 12.]])
        new_rs = space.shift(rs, drs)
        expected_rs = jnp.array([[8., 0., 2.], [4., 6., 8.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_shift_indexes(self):
        """Test JAX PeriodicSpace shift_indexes"""
        space = IMP._jax_util.PeriodicSpace([10., 10., 10.])
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        indexes = jnp.array([0])
        drs = jnp.array([[7., 8., 9.]])
        new_rs = space.shift_indexes(rs, indexes, drs)
        expected_rs = jnp.array([[8., 0., 2.], [4., 5., 6.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))


if __name__ == '__main__':
    IMP.test.main()
