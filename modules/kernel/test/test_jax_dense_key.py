import IMP
import IMP.test
try:
    import jax
    import IMP._jax_util
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_dense_key(self):
        """Test JAX DenseKey class"""
        fk1 = IMP.FloatKey("DenseKey test1")
        fk2 = IMP.FloatKey("DenseKey test2")
        ik = IMP.IntKey("DenseKey test3")

        self.assertRaises(TypeError, IMP._jax_util._DenseKey, ik)

        dk1 = IMP._jax_util._DenseKey(fk1)
        dk1a = IMP._jax_util._DenseKey(fk1)
        self.assertNotEqual(id(dk1), id(dk1a))
        self.assertEqual(hash(dk1), hash(dk1a))
        self.assertEqual(dk1, dk1a)

        dk2 = IMP._jax_util._DenseKey(fk2)
        self.assertNotEqual(dk1, dk2)


if __name__ == '__main__':
    IMP.test.main()
