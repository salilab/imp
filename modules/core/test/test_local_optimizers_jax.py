import IMP
import IMP.test
import IMP.core
import IMP.algebra
try:
    import jax
except ImportError:
    jax = None


class CountingOptimizerState(IMP.OptimizerState):
    """Count updates, and record the score at each update"""

    def __init__(self, m, sf, period=1):
        super().__init__(m, "CountingOptimizerState%1%")
        self.sf = sf
        self.scores = []
        self.set_period(period)

    def do_update(self, call):
        self.scores.append(self.sf.evaluate(False))

    def _get_jax(self, state_index):
        return None


def setup_chain(n=6, fix_first=True):
    """Make a chain of particles connected by harmonic springs, starting
       from random coordinates"""
    IMP.random_number_generator.seed(42)
    m = IMP.Model()
    bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                   IMP.algebra.Vector3D(10, 10, 10))
    ps = []
    for i in range(n):
        d = IMP.core.XYZ.setup_particle(
            IMP.Particle(m), IMP.algebra.get_random_vector_in(bb))
        d.set_coordinates_are_optimized(not (fix_first and i == 0))
        ps.append(d)
    rs = [IMP.core.DistanceRestraint(m, IMP.core.Harmonic(2.0, 1.0), a, b)
          for a, b in zip(ps[:-1], ps[1:])]
    sf = IMP.core.RestraintsScoringFunction(rs)
    return m, ps, sf


def get_coords(ps):
    return [d.get_coordinates() for d in ps]


class Tests(IMP.test.TestCase):

    def run_both(self, make_opt, nsteps):
        """Run the optimizer in C++ and JAX from the same starting point.
           Return the (score, coordinates) for each."""
        results = []
        for use_jax in (False, True):
            m, ps, sf = setup_chain()
            opt = make_opt(m)
            opt.set_scoring_function(sf)
            if use_jax:
                score = opt._optimize_jax(nsteps)
            else:
                score = opt.optimize(nsteps)
            # The returned score should match the model state
            self.assertAlmostEqual(score, sf.evaluate(False), delta=1e-3)
            results.append((score, get_coords(ps)))
        return results

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_steepest_descent(self):
        """Compare JAX and C++ SteepestDescent"""
        def make_opt(m):
            sd = IMP.core.SteepestDescent(m)
            sd.set_step_size(0.1)
            return sd
        (cpp_score, cpp_coords), (jax_score, jax_coords) = self.run_both(
            make_opt, 50)
        self.assertAlmostEqual(cpp_score, jax_score, delta=1e-3)
        for c, j in zip(cpp_coords, jax_coords):
            self.assertLess(IMP.algebra.get_distance(c, j), 1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_steepest_descent_threshold(self):
        """Test JAX SteepestDescent stops at the threshold"""
        m, ps, sf = setup_chain()
        sd = IMP.core.SteepestDescent(m)
        sd.set_scoring_function(sf)
        sd.set_threshold(5.0)
        score = sd._optimize_jax(1000)
        self.assertLess(score, 5.0)
        self.assertGreater(score, 1.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_fixed_particles(self):
        """Non-optimized particles should not move"""
        for cls in (IMP.core.SteepestDescent,):
            m, ps, sf = setup_chain()
            start = ps[0].get_coordinates()
            opt = cls(m)
            opt.set_scoring_function(sf)
            opt._optimize_jax(20)
            self.assertLess(IMP.algebra.get_distance(
                ps[0].get_coordinates(), start), 1e-6)
            self.assertLess(sf.evaluate(False), 5.0)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_float_attribute(self):
        """Test optimization of a non-XYZ attribute"""
        for cls in (IMP.core.SteepestDescent,):
            m = IMP.Model()
            k = IMP.FloatKey("foo")
            p = IMP.Particle(m)
            p.add_attribute(k, 0., True)
            ss = IMP.core.AttributeSingletonScore(IMP.core.Harmonic(5., 1.),
                                                  k)
            r = IMP.core.SingletonRestraint(m, ss, p)
            opt = cls(m)
            opt.set_scoring_function([r])
            score = opt._optimize_jax(100)
            self.assertAlmostEqual(score, 0., delta=1e-3)
            self.assertAlmostEqual(p.get_value(k), 5., delta=1e-2)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_optimizer_states(self):
        """Test that IMP OptimizerStates are called during JAX runs"""
        for cls in (IMP.core.SteepestDescent,):
            m, ps, sf = setup_chain()
            opt = cls(m)
            opt.set_scoring_function(sf)
            os1 = CountingOptimizerState(m, sf)
            os2 = CountingOptimizerState(m, sf, period=3)
            opt.add_optimizer_states([os1, os2])
            score = opt._optimize_jax(50)
            self.assertGreater(len(os1.scores), 3)
            self.assertEqual(len(os2.scores), len(os1.scores) // 3)
            # Scores seen by the OptimizerState should come from the
            # (synced) optimization trajectory, so should go down
            self.assertLess(os1.scores[-1], os1.scores[0])
            self.assertLessEqual(score, os1.scores[0])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_zero_steps(self):
        """JAX SteepestDescent with zero steps should do nothing"""
        m, ps, sf = setup_chain()
        start = get_coords(ps)
        sd = IMP.core.SteepestDescent(m)
        sd.set_scoring_function(sf)
        self.assertAlmostEqual(sd._optimize_jax(0), 0., delta=1e-6)
        for a, b in zip(start, get_coords(ps)):
            self.assertLess(IMP.algebra.get_distance(a, b), 1e-6)


if __name__ == '__main__':
    IMP.test.main()
