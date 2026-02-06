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
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name('mini.pdb'), m)
        IMP.atom.add_dope_score_data(mh)

        ps = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        # JAX currently has no support for ClosePairContainer
        dpc = IMP.container.AllPairContainer(ps)
        dps = IMP.atom.DopePairScore(7.0)
        d = IMP.container.PairsRestraint(dps, dpc)
        score = d.evaluate(False)
        self.assertAlmostEqual(score, 1062.8766, delta=5.0)

        ji = d._get_jax()
        jm = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(jm), score, delta=1.0)


if __name__ == '__main__':
    IMP.test.main()
