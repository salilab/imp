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
        import IMP._jax_util
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_dope_score_data(mh)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        dpc = IMP.container.ClosePairContainer(ps, 7.0, 0.1)
        dps = IMP.atom.DopePairScore(7.0)
        d = IMP.container.PairsRestraint(dps, dpc)
        score = d.evaluate(False)
        self.assertAlmostEqual(score, 1062.8766, delta=5.0)

        ji = d._get_jax(space=IMP._jax_util.FreeSpace)
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(jm), score, delta=1.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_periodic(self):
        """Check JAX implementation of DopePairScore with PBC"""
        import IMP._jax_util
        import jax.numpy as jnp
        space = IMP._jax_util.PeriodicSpace([4., 4., 4.])
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
