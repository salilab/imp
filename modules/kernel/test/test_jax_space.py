import IMP
import IMP.test
try:
    import jax
    import IMP.jax
    import jax.numpy as jnp
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space_distance(self):
        """Test JAX FreeSpace distance"""
        space = IMP.jax.FreeSpace
        drs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        dists = jnp.array([3.74165, 8.77496])
        self.assertTrue(jnp.allclose(space.distance(drs), dists))
        # Should work for a single vector too
        dr = jnp.array([1., 2., 3.])
        self.assertAlmostEqual(space.distance(dr), dists[0], delta=1e-3)

    def _check_min_dist(self, space):
        drs = jnp.array([[1., 2., 3.], [0., 0., 0.], [0.01, 0., 0.]])
        dists = space.distance(drs, min_dist=0.1)
        self.assertTrue(jnp.allclose(dists, jnp.array([3.74165, 0.1, 0.1])))
        # Should work for a single vector too
        self.assertAlmostEqual(
            space.distance(jnp.array([0., 0., 0.]), min_dist=0.1), 0.1,
            delta=1e-6)
        # Gradient should be finite, and zero where the clamp applies
        g = jax.grad(lambda d: jnp.sum(space.distance(d, min_dist=0.1)))(drs)
        self.assertTrue(bool(jnp.all(jnp.isfinite(g))))
        self.assertTrue(jnp.allclose(g[0], drs[0] / 3.74165))
        self.assertTrue(jnp.allclose(g[1:], 0.))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space_distance_min_dist(self):
        """Test JAX FreeSpace distance with min_dist"""
        self._check_min_dist(IMP.jax.FreeSpace)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_distance_min_dist(self):
        """Test JAX PeriodicSpace distance with min_dist"""
        space = IMP.jax.PeriodicSpace([10., 20., 30.])
        self._check_min_dist(space)
        # A vector of exactly one box length is a zero-length displacement
        d = space.distance(jnp.array([10., 0., 0.]), min_dist=0.1)
        self.assertAlmostEqual(d, 0.1, delta=1e-6)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space_shift(self):
        """Test JAX FreeSpace shift"""
        space = IMP.jax.FreeSpace
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        drs = jnp.array([[10., 20., 30.], [40., 50., 60.]])
        new_rs = space.shift(rs, drs)
        expected_rs = jnp.array([[11., 22., 33.], [44., 55., 66.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_free_space_shift_indexes(self):
        """Test JAX FreeSpace shift_indexes"""
        space = IMP.jax.FreeSpace
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        indexes = jnp.array([0])
        drs = jnp.array([[10., 20., 30.]])
        new_rs = space.shift_indexes(rs, indexes, drs)
        expected_rs = jnp.array([[11., 22., 33.], [4., 5., 6.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

        # Test with multi dimensional data
        multi_rs = jnp.array([[[1., 2., 3.], [4., 5., 6.]],
                              [[7., 8., 9.], [10., 11., 12.]]])
        new_rs = space.shift_indexes(multi_rs, indexes, drs)
        expected_rs = jnp.array([[[11., 22., 33.], [4., 5., 6.]],
                                 [[17., 28., 39.], [10., 11., 12.]]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_distance(self):
        """Test JAX PeriodicSpace distance"""
        space = IMP.jax.PeriodicSpace([10., 20., 30.])
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
    def test_free_space_displacement(self):
        """Test JAX FreeSpace displacement"""
        drs = jnp.array([[9., 2., 3.], [24., 5., 6.]])
        self.assertTrue(jnp.allclose(IMP.jax.FreeSpace.displacement(drs), drs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_displacement(self):
        """Test JAX PeriodicSpace displacement"""
        space = IMP.jax.PeriodicSpace([10., 20., 30.])
        # Components less than half the box size should not change
        drs = jnp.array([[1., 2., 3.], [4., -5., 6.]])
        self.assertTrue(jnp.allclose(space.displacement(drs), drs))

        # Components should wrap over half the box size, preserving their sign
        drs = jnp.array([[9., 2., 3.], [24., -15., 26.]])
        expected = jnp.array([[-1., 2., 3.], [4., 5., -4.]])
        self.assertTrue(jnp.allclose(space.displacement(drs), expected))

        # Should work for a single vector too
        dr = jnp.array([9., 2., 3.])
        self.assertTrue(jnp.allclose(space.displacement(dr), expected[0]))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_shift(self):
        """Test JAX PeriodicSpace shift"""
        space = IMP.jax.PeriodicSpace([10., 10., 10.])
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        drs = jnp.array([[7., 8., 9.], [10., 11., 12.]])
        new_rs = space.shift(rs, drs)
        expected_rs = jnp.array([[8., 0., 2.], [4., 6., 8.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_periodic_space_shift_indexes(self):
        """Test JAX PeriodicSpace shift_indexes"""
        space = IMP.jax.PeriodicSpace([10., 10., 10.])
        rs = jnp.array([[1., 2., 3.], [4., 5., 6.]])
        indexes = jnp.array([0])
        drs = jnp.array([[7., 8., 9.]])
        new_rs = space.shift_indexes(rs, indexes, drs)
        expected_rs = jnp.array([[8., 0., 2.], [4., 5., 6.]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))

        # Test with multi dimensional data
        multi_rs = jnp.array([[[1., 2., 3.], [4., 5., 6.]],
                              [[2., 4., 6.], [6., 7., 8.]]])
        new_rs = space.shift_indexes(multi_rs, indexes, drs)
        expected_rs = jnp.array([[[8., 0., 2.], [4., 5., 6.]],
                                 [[9., 2., 5.], [6., 7., 8.]]])
        self.assertTrue(jnp.allclose(new_rs, expected_rs))


if __name__ == '__main__':
    IMP.test.main()
