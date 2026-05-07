import functools
import IMP.test
import IMP.algebra
import IMP.statistics
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def test_circle(self):
        """Test the creation of points on a circle"""
        s2 = IMP.algebra.Sphere2D(IMP.algebra.Vector2D(.5, .5), 1.5)
        h = IMP.statistics.Histogram2D(.1, IMP.algebra.get_bounding_box(s2))
        for i in range(0, 10000):
            pt = IMP.algebra.get_random_vector_on(s2)
            pt3 = IMP.algebra.Vector3D(pt[0], pt[1], 0)
            h.add(pt)
        center = h.get_mean()
        std = h.get_standard_deviation(center)
        print(center, std)
        for i in range(0, 2):
            self.assertAlmostEqual(center[i], .5, delta=.05)
            self.assertAlmostEqual(std[i], .7 * 1.5, delta=.05)

    def test_sphere(self):
        """Test the creation of points on a sphere"""
        s2 = IMP.algebra.Sphere3D(IMP.algebra.Vector3D(.75, .75, .75), .7)
        h = IMP.statistics.Histogram3D(.1, IMP.algebra.get_bounding_box(s2))
        for i in range(0, 10000):
            pt = IMP.algebra.get_random_vector_on(s2)
            h.add(pt)
        center = h.get_mean()
        std = h.get_standard_deviation(center)
        print(center, std)
        for i in range(0, 3):
            self.assertAlmostEqual(center[i], .75, delta=.017)
            self.assertAlmostEqual(std[i], .58 * .7, delta=.05)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_sphere_jax(self):
        """Test the JAX creation of points on a sphere"""
        from IMP.algebra import _jax_util
        import jax.numpy as jnp
        j = jax.jit(functools.partial(_jax_util.get_random_vector_on_3d_sphere,
                                      shape=20000))
        k = jax.random.key(42)
        v = j(k, radius=1.0)
        self.assertTrue(jnp.allclose(v.mean(axis=0), jnp.zeros(3), atol=0.01))
        self.assertTrue(jnp.allclose(v.std(axis=0), jnp.full(3, 0.58),
                                     atol=0.01))


if __name__ == '__main__':
    IMP.test.main()
