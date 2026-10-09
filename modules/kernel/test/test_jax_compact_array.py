import IMP
import IMP.test
try:
    import jax
    import IMP.jax
except ImportError:
    jax = None


fk = IMP.FloatKey("CompactArray test")


def _make_model(multi=False):
    m = IMP.Model()
    for _ in range(10):
        p = IMP.Particle(m)
    p1 = IMP.Particle(m)
    for _ in range(5):
        p = IMP.Particle(m)
    p2 = IMP.Particle(m)
    # Set up attribute on p2 first so that the array gets the full dimension
    m.add_attribute(fk, p2.get_index(), 9.0)
    if multi:
        m.set_number_of_attribute_sets(fk, 2)
        m.set_active_attribute_set(fk, 1)
    m.add_attribute(fk, p1.get_index(), 2.0)

    return m, p1, p2


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_compact_array(self):
        """Test JAX CompactArray class"""
        import numpy as np
        # Make a Model with fk particles at indexes 10, 16
        m, p1, p2 = _make_model()

        ca = IMP.jax._CompactArray.from_model(m, fk)
        # data should include the two particles with the attribute plus
        # an 'inf' row for out of range indexes
        self.assertEqual(ca.data.shape, (3,))
        self.assertEqual(ca.remap.indexes.shape, (2,))
        self.assertEqual(ca.remap.mapping.shape, (17,))
        self.assertEqual(ca.remap.full_view.shape, (17,))
        self.assertFalse(ca.remap.full_view.flags.owndata)

        # We should be able to read CompactArray like any other JAX PyTree
        # using original indexes
        def get_sum(a):
            return a[10] + a[16]

        f = jax.jit(get_sum)
        self.assertAlmostEqual(f(ca), 11.0, delta=1e-4)

        # Particles that don't have the attribute should return inf, just
        # as if we access the original NumPy array directly
        def get_bad_particle(a):
            return a[0]
        f = jax.jit(get_bad_particle)
        self.assertEqual(f(ca), np.inf)

        # We should be able to modify CompactArray using at()
        def modify_a(a):
            return a.at[10].set(42.0)

        f = jax.jit(modify_a)
        ca = f(ca)
        self.assertAlmostEqual(ca[10], 42.0, delta=1e-4)

        def add_to_a(a):
            return a.at[10].add(20.0)

        f = jax.jit(add_to_a)
        ca = f(ca)
        self.assertAlmostEqual(ca[10], 62.0, delta=1e-4)

        # Only JAX arrays should be changed, not the original IMP data
        self.assertAlmostEqual(m.get_attribute(fk, p1.get_index()), 2.0,
                               delta=1e-4)
        self.assertAlmostEqual(m.get_attribute(fk, p2.get_index()), 9.0,
                               delta=1e-4)

        # We should be able to sync back to the IMP original array
        ca.sync()
        self.assertAlmostEqual(m.get_attribute(fk, p1.get_index()), 62.0,
                               delta=1e-4)
        self.assertAlmostEqual(m.get_attribute(fk, p2.get_index()), 9.0,
                               delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_compact_array_multi(self):
        """Test JAX CompactArray class with multiple attribute sets"""
        import numpy as np
        # Make a Model with fk particles at indexes 10, 16
        # in two attribute sets
        m, p1, p2 = _make_model(multi=True)

        ca = IMP.jax._CompactArray.from_model(m, fk)
        self.assertEqual(ca.data.shape, (2, 3))
        self.assertEqual(ca.remap.indexes.shape, (2,))
        self.assertEqual(ca.remap.mapping.shape, (17,))
        self.assertEqual(ca.remap.full_view.shape, (2, 17))
        self.assertFalse(ca.remap.full_view.flags.owndata)

        def get_sum(a):
            return a[1,10] + a[0,16]

        f = jax.jit(get_sum)
        self.assertAlmostEqual(f(ca), 11.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
