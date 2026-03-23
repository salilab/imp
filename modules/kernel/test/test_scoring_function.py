import sys
import IMP
import IMP.test
import IMP.core
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    """Tests for ScoreState"""

    def test_score_state_show(self):
        """Test scoring function linkage"""
        m = IMP.Model("scoring function linkage")
        ps = [IMP.Particle(m) for i in range(0, 10)]
        r = IMP._ConstRestraint(m, ps, 1)
        r.create_scoring_function()
        self.assertEqual(r.evaluate(False), 1)

    def test_reweighting(self):
        """Test scoring function reweighting"""
        m = IMP.Model("scoring function linkage")
        ps = [IMP.Particle(m) for i in range(0, 10)]
        r = IMP._ConstRestraint(m, ps, 1)
        rs = IMP.RestraintSet(m, 1.0, "rs")
        rs.add_restraint(r)
        rsf = rs.create_scoring_function()
        self.assertEqual(rsf.evaluate(False), 1)
        r.set_weight(0)
        self.assertEqual(rsf.evaluate(False), 0)
        r.set_weight(1)
        rs.set_weight(0)
        self.assertEqual(rsf.evaluate(False), 0)

    def test_implicit_derived_object(self):
        """Test get_derived_object() of implicit RestraintScoringFunction"""
        m = IMP.Model()
        r = IMP.RestraintSet(m)
        sf = r.create_scoring_function()
        # No Python class for implicit scoring functions, so
        # get_derived_object() should return the base class
        self.assertIs(type(sf.get_derived_object()), IMP.ScoringFunction)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_single_score(self):
        """Test JAX score of implicit single RestraintScoringFunction"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r1 = IMP._ConstRestraint(m, [p], 42)
        r1.set_weight(2.0)
        r2 = IMP._ConstRestraint(m, [p], 18)
        r2.set_weight(3.0)
        r = IMP.RestraintSet(m)
        r.set_weight(4.0)
        r.add_restraints([r1, r2])
        sf = r.create_scoring_function()

        ji = sf._get_jax()
        X = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(X), 552.0, delta=0.1)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multiple_score(self):
        """Test JAX score of implicit multiple RestraintsScoringFunction"""
        m = IMP.Model()
        p = IMP.Particle(m)
        r1 = IMP._ConstRestraint(m, [p], 42)
        r1.set_weight(2.0)
        r2 = IMP._ConstRestraint(m, [p], 18)
        r2.set_weight(3.0)
        c = IMP.core.ConjugateGradients()
        c.set_scoring_function([r1, r2])
        sf = c.get_scoring_function().get_derived_object()
        self.assertIsInstance(sf, IMP._RestraintsScoringFunction)

        ji = sf._get_jax()
        X = ji.get_jax_model()
        j = jax.jit(ji.score_func)
        self.assertAlmostEqual(j(X), 138.0, delta=0.1)


if __name__ == '__main__':
    IMP.test.main()
