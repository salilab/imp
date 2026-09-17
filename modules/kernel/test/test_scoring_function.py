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

    def _make_jax_distance_restraint(self):
        m = IMP.Model()
        p1 = IMP.Particle(m)
        _ = IMP.core.XYZ.setup_particle(p1, IMP.algebra.Vector3D(0, 0, 0))
        p2 = IMP.Particle(m)
        _ = IMP.core.XYZ.setup_particle(p2, IMP.algebra.Vector3D(7, 0, 0))
        uf = IMP.core.Linear(0.0, 1.0)
        r = IMP.core.DistanceRestraint(m, uf, p1, p2)
        rs = IMP.RestraintSet(m, 1.0)
        rs.add_restraint(r)
        return m, r, rs

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_single_score_periodic(self):
        """Test JAX PBC score of implicit single RestraintScoringFunction"""
        import IMP._jax_util
        space = IMP._jax_util.PeriodicSpace([10., 10., 10.])
        # Returns two particles at 0,0,0 and 7,0,0
        m, r, rs = self._make_jax_distance_restraint()
        # In periodic space, distance is 3.0, not 7.0
        # Test scoring function created from RestraintSet
        sf = rs.create_scoring_function().get_derived_object()
        self.assertIs(type(sf), IMP.ScoringFunction)
        self.assertAlmostEqual(sf._evaluate_jax(space=space), 3.0, delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multiple_score_periodic(self):
        """Test JAX PBC score of implicit multiple RestraintsScoringFunction"""
        import IMP._jax_util
        space = IMP._jax_util.PeriodicSpace([10., 10., 10.])
        # Returns two particles at 0,0,0 and 7,0,0
        m, r, rs = self._make_jax_distance_restraint()

        c = IMP.core.ConjugateGradients()
        c.set_scoring_function([r, r])
        sf = c.get_scoring_function().get_derived_object()
        self.assertIsInstance(sf, IMP._RestraintsScoringFunction)

        # In periodic space, distance is 3.0, not 7.0, but there are
        # two restraints
        self.assertAlmostEqual(sf._evaluate_jax(space=space), 6.0, delta=1e-3)


if __name__ == '__main__':
    IMP.test.main()
