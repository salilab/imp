import IMP
import IMP.test
import numpy
try:
    import jax
    import jax.numpy as jnp
    import jax.test_util
    import IMP.isd._jax_util
except ImportError:
    jax = None


@IMP.test.skipIf(jax is None, "No JAX support")
class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.segments = numpy.array([0, 0, 0, 1, 2, 2, 3, 3, 3, 3])
        self.segment_prod = IMP.isd._jax_util.get_segment_prod(
            self.segments, 4)
        rng = numpy.random.default_rng(42)
        self.data = jnp.array(rng.uniform(0.1, 1.0, len(self.segments)))

    def assert_close(self, a, b):
        self.assertTrue(numpy.allclose(a, b, rtol=1e-5, atol=1e-6),
                        "%s != %s" % (a, b))

    def test_value(self):
        """Test value of differentiable segment_prod"""
        self.assert_close(
            self.segment_prod(self.data),
            jax.ops.segment_prod(self.data, self.segments, 4))
        data2 = jnp.stack((self.data, 2. * self.data), axis=1)
        self.assert_close(
            self.segment_prod(data2),
            jax.ops.segment_prod(data2, self.segments, 4))

    def test_derivatives(self):
        """Test derivatives of differentiable segment_prod"""
        def ref(data):
            # Correct only for positive data
            return jnp.exp(jax.ops.segment_sum(jnp.log(data),
                                               self.segments, 4))
        for jac in (jax.jacfwd, jax.jacrev):
            self.assert_close(jac(self.segment_prod)(self.data),
                              jac(ref)(self.data))
        data2 = jnp.stack((self.data, 2. * self.data), axis=1)
        self.assert_close(jax.jacrev(self.segment_prod)(data2),
                          jax.jacrev(ref)(data2))
        self.assert_close(jax.jit(jax.jacrev(self.segment_prod))(self.data),
                          jax.jacrev(ref)(self.data))
        # Second derivatives
        self.assert_close(jax.hessian(self.segment_prod)(self.data),
                          jax.hessian(ref)(self.data))

    def test_derivatives_zero(self):
        """Test derivatives of differentiable segment_prod with zeros"""
        # Single zero in segment 0; two zeros in segment 3
        data = self.data.at[1].set(0.).at[6].set(0.).at[8].set(0.)
        jac = jax.jacrev(self.segment_prod)(data)
        self.assertTrue(bool(jnp.all(jnp.isfinite(jac))))
        self.assert_close(jac[0, :3],
                          [0., data[0] * data[2], 0.])
        self.assert_close(jac[3, 6:], [0., 0., 0., 0.])
        self.assert_close(jac[2, 4:6], [data[5], data[4]])
        self.assert_close(jax.jacfwd(self.segment_prod)(data), jac)


if __name__ == '__main__':
    IMP.test.main()
