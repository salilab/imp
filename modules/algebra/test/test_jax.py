import IMP.test
import IMP.algebra
try:
    import jax
    import jax.numpy as jnp
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def test_quaternion_normalize(self):
        """Test _quaternion_normalize()"""
        from IMP.algebra.jax import _quaternion_normalize
        # Single quaternion
        q = jnp.array([0.3, 0.3, 0.3, 0.3])
        self.assertTrue(jnp.allclose(_quaternion_normalize(q),
                                     jnp.array([0.5, 0.5, 0.5, 0.5])))
        q = jnp.array([0.0, 0.0, 0.0, 0.0])
        self.assertTrue(jnp.allclose(_quaternion_normalize(q),
                                     jnp.array([1.0, 0.0, 0.0, 0.0])))

        # Array of quaternions
        q = jnp.array([[0.3, 0.3, 0.3, 0.3], [1.0, 0.0, 0.0, 0.0],
                       [0.0, 0.0, 0.0, 0.0]])
        self.assertTrue(jnp.allclose(_quaternion_normalize(q),
                                     jnp.array([[0.5, 0.5, 0.5, 0.5],
                                                [1., 0., 0., 0.],
                                                [1., 0., 0., 0.]])))


if __name__ == '__main__':
    IMP.test.main()
