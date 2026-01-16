import IMP
import IMP.test
import IMP.atom
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    def test_removal(self):
        """Check removal of CenterOfMass decorator"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        xyz1 = IMP.core.XYZ.setup_particle(p1, [0, 0, 0])
        mass1 = IMP.atom.Mass.setup_particle(p1, 1.0)

        p2 = IMP.Particle(m)
        com = IMP.atom.CenterOfMass.setup_particle(p2, [p1])
        # Should be a ScoreState to update COM
        self.assertEqual(len(m.get_score_states()), 1)
        self.assertEqual(len(m.get_particle_indexes()), 2)

        # Remove decorator and ScoreState
        IMP.atom.CenterOfMass.teardown_particle(com)
        self.assertEqual(len(m.get_score_states()), 0)
        m.update()

        # It should now be OK to remove the particle too
        m.remove_particle(p2)
        self.assertEqual(len(m.get_particle_indexes()), 1)
        m.update()

    def test_constraint_accessors(self):
        """Check accessors of CenterOfMass-created constraint"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        xyz1 = IMP.core.XYZ.setup_particle(p1, [0, 0, 0])
        mass1 = IMP.atom.Mass.setup_particle(p1, 1.0)

        p2 = IMP.Particle(m)
        com = IMP.atom.CenterOfMass.setup_particle(p2, [p1])
        ss = m.score_states[0].get_derived_object()
        self.assertIsInstance(ss, IMP.core.SingletonConstraint)
        bm = ss.get_before_modifier().get_derived_object()
        self.assertIsInstance(bm, IMP.core.CentroidOfRefined)
        self.assertFalse(bm.get_is_weight_null())
        weight = bm.get_weight()
        self.assertEqual(weight.get_string(), "mass")
        refiner = bm.get_refiner()
        self.assertIsInstance(refiner, IMP.Refiner)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Test JAX implememtation of CenterOfMass constraint"""
        import jax.numpy as jnp
        m = IMP.Model()
        ps = IMP.core.create_xyzr_particles(m, 10, 1)
        for i, p in enumerate(ps):
            IMP.atom.Mass.setup_particle(p, 10.0 * (i + 1))

        p2 = IMP.Particle(m)
        com = IMP.atom.CenterOfMass.setup_particle(p2, ps)

        # Get JAX implementation of COM
        ji = m.get_score_states()[0].get_derived_object()._get_jax()
        jm = ji.get_jax_model()

        # Make model mutable
        jm['xyz'] = jnp.asarray(jm['xyz'])

        # Get coordinates for p2 using JAX
        jm = ji.apply_func(jm)
        jax_coord = IMP.algebra.Vector3D(jm['xyz'][-1])

        # Compare with p2's coordinates using IMP
        m.update()
        imp_coord = IMP.core.XYZ(p2).get_coordinates()
        self.assertLess(IMP.algebra.get_distance(jax_coord, imp_coord), 0.01)


if __name__ == '__main__':
    IMP.test.main()
