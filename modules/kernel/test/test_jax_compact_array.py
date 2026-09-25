import IMP
import IMP.test
try:
    import jax
    import IMP.jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_compact_array(self):
        """Test JAX CompactArray class"""
        fk = IMP.FloatKey("CompactArray test")
        # Make a Model with fk particles at indexes 10, 16
        m = IMP.Model()
        for _ in range(10):
            p = IMP.Particle(m)
        p1 = IMP.Particle(m)
        m.add_attribute(fk, p1.get_index(), 2.0)
        for _ in range(5):
            p = IMP.Particle(m)
        p2 = IMP.Particle(m)
        m.add_attribute(fk, p2.get_index(), 9.0)

        ca = IMP.jax._CompactArray.from_model(m, fk)
        self.assertEqual(ca.data.shape, (2,))
        self.assertEqual(ca.indexes.shape, (2,))
        self.assertEqual(ca.remap.shape, (17,))
        self.assertEqual(ca.full_view.shape, (17,))
        self.assertFalse(ca.full_view.flags.owndata)

        # We should be able to read CompactArray like any other JAX PyTree
        # using original indexes
        def get_sum(a):
            return a[10] + a[16]

        f = jax.jit(get_sum)
        self.assertAlmostEqual(f(ca), 11.0, delta=1e-4)

        # We should be able to modify CompactArray using at()
        def modify_a(a):
            return a.at[10].set(42.0)

        f = jax.jit(modify_a)
        ca = f(ca)
        self.assertAlmostEqual(ca[10], 42.0, delta=1e-4)

        # Only JAX arrays should be changed, not the original IMP data
        self.assertAlmostEqual(m.get_attribute(fk, p1.get_index()), 2.0,
                               delta=1e-4)
        self.assertAlmostEqual(m.get_attribute(fk, p2.get_index()), 9.0,
                               delta=1e-4)

        # We should be able to sync back to the IMP original array
        ca.sync()
        self.assertAlmostEqual(m.get_attribute(fk, p1.get_index()), 42.0,
                               delta=1e-4)
        self.assertAlmostEqual(m.get_attribute(fk, p2.get_index()), 9.0,
                               delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
