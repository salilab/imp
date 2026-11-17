import IMP.test
import IMP.algebra
try:
    import jax
    import jax.numpy as jnp
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
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

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_transformation_precision(self):
        """Test Transformation3DWithMatrix.get_transformed() precision"""
        import numpy as np
        from IMP.algebra.jax import Transformation3D, _quaternion_normalize
        # Use enough vectors (>30) that XLA on GPU would use a GEMM kernel
        # for a matrix product, which by default rounds float32 operands
        # to TF32 (see https://github.com/salilab/imp/issues/1129)
        rng = np.random.default_rng(42)
        q = rng.normal(size=4)
        q /= np.linalg.norm(q)
        tr = np.array([150., -200., 120.])
        v = rng.uniform(-30., 30., size=(100, 3))
        t = Transformation3D(
            rotation=_quaternion_normalize(jnp.array(q, dtype=jnp.float32)),
            translation=jnp.array(tr, dtype=jnp.float32))
        tv = jax.jit(lambda t, v: t.get_with_matrix().get_transformed(v))(
            t, jnp.array(v, dtype=jnp.float32))
        self.assertEqual(tv.dtype, jnp.float32)
        # Reference, using IMP's (double precision) C++ implementation
        # with the same float32 inputs
        rq = IMP.algebra.Rotation3D(
            IMP.algebra.Vector4D(*np.asarray(t.rotation, dtype=np.float64)))
        rt = IMP.algebra.Transformation3D(
            rq, IMP.algebra.Vector3D(*np.asarray(t.translation,
                                                 dtype=np.float64)))
        v32 = np.asarray(jnp.array(v, dtype=jnp.float32), dtype=np.float64)
        ref = np.array([rt.get_transformed(IMP.algebra.Vector3D(*x))
                        for x in v32])
        # Error should be on the order of float32 epsilon times the
        # magnitude of the result (TF32 would give errors ~1e3x larger)
        tol = 16. * np.finfo(np.float32).eps * np.max(np.abs(ref))
        err = np.max(np.abs(np.asarray(tv, dtype=np.float64) - ref))
        self.assertLess(err, tol)


if __name__ == '__main__':
    IMP.test.main()
