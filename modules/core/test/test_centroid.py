import IMP
import IMP.atom
import IMP.test
import IMP.core
import IMP.algebra
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    """Tests for RefinerCover"""

    def test_it(self):
        """Test centroid of refined decorator"""
        m = IMP.Model()
        ps = IMP.core.create_xyzr_particles(m, 10, 1)
        fpr = IMP.core.FixedRefiner(ps)
        p = IMP.core.Centroid.setup_particle(IMP.Particle(m), fpr)
        m.update()

        ss = m.score_states[0].get_derived_object()
        self.assertIsInstance(ss, IMP.core.SingletonConstraint)
        bm = ss.get_before_modifier().get_derived_object()
        self.assertIsInstance(bm, IMP.core.CentroidOfRefined)
        self.assertTrue(bm.get_is_weight_null())
        refiner = bm.get_refiner()
        self.assertIsInstance(refiner, IMP.Refiner)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implementation of centroid"""
        import jax.numpy as jnp
        m = IMP.Model()
        ps = IMP.core.create_xyzr_particles(m, 10, 1)
        for p in ps:
            IMP.atom.Mass.setup_particle(p, 100.0)
        fpr = IMP.core.FixedRefiner(ps)
        p = IMP.core.Centroid.setup_particle(IMP.Particle(m), fpr)

        # Get JAX implementation of centroid
        ji = m.get_score_states()[0].get_derived_object()._get_jax()
        jm = ji.get_jax_model()

        # Make model mutable
        jm['xyz'] = jnp.asarray(jm['xyz'])

        # Get coordinates for p using JAX
        j = jax.jit(ji.apply_func)
        jm = j(jm)
        jax_coord = IMP.algebra.Vector3D(jm['xyz'][-1])

        # Compare with p's coordinates using IMP
        m.update()
        imp_coord = IMP.core.XYZ(p).get_coordinates()
        self.assertLess(IMP.algebra.get_distance(jax_coord, imp_coord), 0.01)


if __name__ == '__main__':
    IMP.test.main()
