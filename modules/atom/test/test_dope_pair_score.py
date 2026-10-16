import IMP
import IMP.test
import IMP.atom
import IMP.container
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    """Test the DopePairScore"""

    def test_value(self):
        """Check score value of DopePairScore"""
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_dope_score_data(mh)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        dpc = IMP.container.ClosePairContainer(ps, 7.0, 0.1)
        dps = IMP.atom.DopePairScore(7.0)
        d = IMP.container.PairsRestraint(dps, dpc)
        score = d.evaluate(False)
        self.assertAlmostEqual(score, 1062.8766, delta=5.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """Check JAX implementation of DopePairScore"""
        import IMP.jax
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_dope_score_data(mh)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        dpc = IMP.container.ClosePairContainer(ps, 7.0, 0.1)
        dps = IMP.atom.DopePairScore(7.0)
        d = IMP.container.PairsRestraint(dps, dpc)
        score = d.evaluate(False)
        self.assertAlmostEqual(score, 1062.8766, delta=5.0)

        ji = d._get_jax(space=IMP.jax.FreeSpace)
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(jm), score, delta=1.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multi_attr(self):
        """Check JAX implementation of DopePairScore, multiple attrsets"""
        import IMP.jax
        m = IMP.Model()

        # First attribute set
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_dope_score_data(mh)
        p1, p2 = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)[:2]

        dps = IMP.atom.DopePairScore(7.0)
        d = IMP.core.PairRestraint(m, dps, (p1, p2))
        imp_score1 = d.evaluate(False)

        # Second attribute set
        m.set_number_of_sphere_attribute_sets(2)
        m.set_active_sphere_attribute_set(1)
        d1 = IMP.core.XYZ.setup_particle(p1)
        d2 = IMP.core.XYZ.setup_particle(p2, (4.0, 1.0, 0.0))
        imp_score2 = d.evaluate(False)

        jax_score = d._evaluate_jax()
        self.assertEqual(jax_score.shape, (2,))
        self.assertAlmostEqual(jax_score[0], imp_score1, delta=1e-4)
        self.assertAlmostEqual(jax_score[1], imp_score2, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_periodic(self):
        """Check JAX implementation of DopePairScore with PBC"""
        import IMP.jax
        import jax.numpy as jnp
        space = IMP.jax.PeriodicSpace([4., 4., 4.])
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_dope_score_data(mh)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        dps = IMP.atom.DopePairScore(7.0)

        ji = dps._get_jax(
            m, jnp.array([[ps[0].get_particle_index(),
                           ps[3].get_particle_index()]]), space=space)
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(jm), 7.089416, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
