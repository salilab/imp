import math
import os
import random
import IMP
import IMP.algebra
import IMP.atom
import IMP.core
import IMP.test
import IMP.pmi.restraints.basic
from IMP.pmi.restraints.basic import PMFRestraint
try:
    import jax
except ImportError:
    jax = None

# A smooth test PMF (in kT): a well of depth DEPTH at R0 on top of a
# repulsive term near the start of the table
RMIN, RMAX, R0, DEPTH, WIDTH = 90., 700., 150., 1.5, 25.


def analytic_pmf(r):
    return (-DEPTH * math.exp(-(r - R0) ** 2 / (2 * WIDTH ** 2))
            + 2.0 * math.exp(-(r - RMIN) / 10.))


def write_table(fname, f=analytic_pmf, spacing=0.5, scale=1.0, header=True):
    with open(fname, 'w') as fh:
        if header:
            fh.write("# Test PMF\n#\nDistance_Angstrom,PMF\n")
        n = int(round((RMAX - RMIN) / spacing))
        for i in range(n + 1):
            r = RMIN + i * spacing
            fh.write("%.6f,%.12f\n" % (r, f(r) * scale))


def score(r):
    """Score a PMI restraint through a scoring function, which (unlike
       RestraintBase.evaluate()) first updates the close pair container"""
    sf = IMP.core.RestraintsScoringFunction([r.get_restraint()])
    return sf.evaluate(False)


def make_particles(m, coords):
    ps = []
    for c in coords:
        p = IMP.Particle(m)
        IMP.core.XYZR.setup_particle(
            p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(*c), 1.0))
        ps.append(p)
    return ps


class Tests(IMP.test.TestCase):

    def make_restraint(self, coords, cutoff=400., **kwargs):
        m = IMP.Model()
        ps = make_particles(m, coords)
        fname = self.get_tmp_file_name('pmf.csv')
        write_table(fname)
        kwargs.setdefault('units', 'kT')
        r = PMFRestraint(ps, fname, cutoff, **kwargs)
        return m, ps, r

    def expected(self, r, cutoff=400.):
        """What the restraint should give for one pair at distance r"""
        if r >= cutoff:
            return 0.
        return analytic_pmf(max(r, RMIN)) - analytic_pmf(cutoff)

    def test_read_table(self):
        """Comments, header and extra columns are skipped; rows sorted"""
        fname = self.get_tmp_file_name('pmf.csv')
        with open(fname, 'w') as fh:
            fh.write("# comment\nr,W,extra\n\n3.0,0.5,9\n1.0,2.0,9\n"
                     "2.0 1.0\n")
        r, w = PMFRestraint.read_table(fname)
        self.assertEqual(r, [1.0, 2.0, 3.0])
        self.assertEqual(w, [2.0, 1.0, 0.5])

    def test_read_table_errors(self):
        """Empty tables and duplicate distances are rejected"""
        fname = self.get_tmp_file_name('pmf.csv')
        with open(fname, 'w') as fh:
            fh.write("# nothing\nr,W\n1.0,2.0\n")
        self.assertRaises(ValueError, PMFRestraint.read_table, fname)
        with open(fname, 'w') as fh:
            fh.write("1.0,2.0\n1.0,3.0\n2.0,0.0\n")
        self.assertRaises(ValueError, PMFRestraint.read_table, fname)

    def test_tabulate(self):
        """Grid ends exactly on the cutoff, where the PMF is zero"""
        r = [0., 1., 2., 3., 4.]
        w = [4., 3., 2., 1., 0.5]
        rmin, spacing, pmf = PMFRestraint.tabulate(r, w, 2.5, 'kT',
                                                   spacing=0.3)
        self.assertAlmostEqual(rmin, 0., delta=1e-12)
        self.assertAlmostEqual(rmin + spacing * (len(pmf) - 1), 2.5,
                               delta=1e-9)
        self.assertAlmostEqual(pmf[-1], 0., delta=1e-12)
        # W(0) - W(2.5) = 4 - 1.5
        self.assertAlmostEqual(pmf[0], 2.5, delta=1e-12)

    def test_tabulate_errors(self):
        """Bad cutoffs, units and missing temperature are rejected"""
        r, w = [0., 1., 2.], [1., 0., 0.]
        tabulate = PMFRestraint.tabulate
        self.assertRaises(ValueError, tabulate, r, w, 3., 'kT')
        self.assertRaises(ValueError, tabulate, r, w, 0., 'kT')
        self.assertRaises(ValueError, tabulate, r, w, 1., 'eV')
        self.assertRaises(ValueError, tabulate, r, w, 1., 'kJ/mol')

    def test_unit_conversion(self):
        """A PMF in kJ/mol or kcal/mol is converted to kT"""
        T = 310.
        for units, R in (('kJ/mol', 8.314462618e-3),
                         ('kcal/mol', 1.987204259e-3)):
            m = IMP.Model()
            ps = make_particles(m, [(0, 0, 0), (R0, 0, 0)])
            fname = self.get_tmp_file_name('pmf.csv')
            write_table(fname, scale=R * T)
            r = PMFRestraint(ps, fname, 400., units, temperature=T)
            self.assertAlmostEqual(score(r), self.expected(R0),
                                   delta=1e-6)

    def test_pair_score(self):
        """A single pair is scored with W(r) - W(cutoff), in kT"""
        m, ps, r = self.make_restraint([(0, 0, 0), (100, 0, 0)])
        xyz = IMP.core.XYZ(ps[1])
        for d in (91., 100., 123.4, R0, 200., 350., 399.):
            xyz.set_coordinates(IMP.algebra.Vector3D(d, 0, 0))
            self.assertAlmostEqual(score(r), self.expected(d),
                                   delta=1e-5)
            self.assertAlmostEqual(r.get_pmf(d), self.expected(d),
                                   delta=1e-5)

    def test_cutoff(self):
        """Score is zero beyond the cutoff and continuous at it"""
        m, ps, r = self.make_restraint([(0, 0, 0), (100, 0, 0)])
        xyz = IMP.core.XYZ(ps[1])
        for d in (399.999, 400.001, 420., 450., 2000.):
            xyz.set_coordinates(IMP.algebra.Vector3D(d, 0, 0))
            self.assertAlmostEqual(score(r), 0., delta=1e-4)

    def test_below_table(self):
        """Below the table the PMF is held constant (no wall)"""
        m, ps, r = self.make_restraint([(0, 0, 0), (50, 0, 0)])
        self.assertAlmostEqual(score(r), self.expected(RMIN),
                               delta=1e-5)

    def test_derivatives(self):
        """Analytic derivatives match numerical ones"""
        m, ps, r = self.make_restraint(
            [(0, 0, 0), (120, 30, 0), (60, 170, 5), (250, 140, -10)])
        sf = IMP.core.RestraintsScoringFunction([r.get_restraint()])
        for p in ps:
            self.assertXYZDerivativesInTolerance(
                sf, IMP.core.XYZ(p), 1e-3, 1e-2)

    def test_sum_over_pairs(self):
        """Score of many particles is the sum over all pairs in range"""
        rng = random.Random(42)
        coords = [(rng.uniform(0, 1500), rng.uniform(0, 1500), 0.)
                  for _ in range(40)]
        m, ps, r = self.make_restraint(coords)
        expected = 0.
        for i in range(len(coords)):
            for j in range(i + 1, len(coords)):
                expected += self.expected(math.dist(coords[i], coords[j]))
        # loose tolerance: natural spline is a little off at the table start
        self.assertAlmostEqual(score(r), expected, delta=1e-3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax(self):
        """JAX score and gradient match the C++ implementation"""
        rng = random.Random(3)
        coords = [(rng.uniform(0, 1000), rng.uniform(0, 1000), 0.)
                  for _ in range(15)]
        m, ps, r = self.make_restraint(coords)
        sf = IMP.core.RestraintsScoringFunction([r.get_restraint()])
        cpp_score = sf.evaluate(True)
        ji = sf._get_jax()
        X = ji.get_jax_model()
        self.assertAlmostEqual(float(jax.jit(ji.score_func)(X)), cpp_score,
                               delta=1e-4)
        grad = jax.grad(ji.score_func)(X)['xyz']
        for p in ps:
            d = IMP.core.XYZ(p).get_derivatives()
            g = grad[int(p.get_index())]
            for k in range(3):
                self.assertAlmostEqual(float(g[k]), d[k], delta=1e-4)

    def test_pairs_follow_particles(self):
        """Pairs are updated as particles move into and out of range"""
        m, ps, r = self.make_restraint(
            [(0, 0, 0), (3000, 0, 0), (6000, 0, 0)])
        sf = IMP.core.RestraintsScoringFunction([r.get_restraint()])
        self.assertAlmostEqual(sf.evaluate(False), 0., delta=1e-8)
        IMP.core.XYZ(ps[1]).set_coordinates(IMP.algebra.Vector3D(R0, 0, 0))
        self.assertAlmostEqual(sf.evaluate(False), self.expected(R0),
                               delta=1e-5)
        IMP.core.XYZ(ps[2]).set_coordinates(
            IMP.algebra.Vector3D(R0, 200, 0))
        self.assertAlmostEqual(
            sf.evaluate(False),
            self.expected(R0) + self.expected(200.)
            + self.expected(math.hypot(R0, 200.)), delta=1e-5)
        IMP.core.XYZ(ps[1]).set_coordinates(
            IMP.algebra.Vector3D(-5000, 0, 0))
        self.assertAlmostEqual(sf.evaluate(False),
                               self.expected(math.hypot(R0, 200.)),
                               delta=1e-5)

    def test_weight_and_output(self):
        """Weight is applied once, in both evaluate() and get_output()"""
        m, ps, r = self.make_restraint([(0, 0, 0), (R0, 0, 0)],
                                       weight=2.0, label='test')
        r.add_to_model()
        self.assertAlmostEqual(score(r), 2 * self.expected(R0),
                               delta=1e-5)
        out = r.get_output()(None)
        self.assertAlmostEqual(float(out['_TotalScore']),
                               2 * self.expected(R0), delta=1e-5)
        self.assertIn('PMFRestraint_Score_test', out)

    def test_hierarchy_input(self):
        """Hierarchies are accepted as well as particles"""
        m = IMP.Model()
        ps = make_particles(m, [(0, 0, 0), (R0, 0, 0)])
        hs = [IMP.atom.Hierarchy.setup_particle(p) for p in ps]
        fname = self.get_tmp_file_name('pmf.csv')
        write_table(fname)
        r = PMFRestraint(hs, fname, 400., 'kT')
        self.assertAlmostEqual(score(r), self.expected(R0), delta=1e-5)

    def test_bad_input(self):
        """Too few particles, or particles without coordinates"""
        m = IMP.Model()
        fname = self.get_tmp_file_name('pmf.csv')
        write_table(fname)
        ps = make_particles(m, [(0, 0, 0)])
        self.assertRaises(ValueError, PMFRestraint, ps, fname, 400., 'kT')
        ps.append(IMP.Particle(m))
        self.assertRaises(TypeError, PMFRestraint, ps, fname, 400., 'kT')

    def test_two_body_sampling(self):
        """Monte Carlo at kT=1 samples an isolated pair with p(r)~r g(r)

        This checks the sign and the units of the restraint: the
        distance distribution of one pair in the plane must be
        2 pi r exp(-W(r)/kT) (times the confining wall)."""
        IMP.random_number_generator.seed(7)
        cutoff, rwall, kwall = 400., 450., 0.05
        m, ps, r = self.make_restraint([(0, 0, 0), (R0, 0, 0)],
                                       cutoff=cutoff)
        origin = IMP.algebra.Vector3D(0, 0, 0)
        wall = IMP.core.SingletonRestraint(
            m, IMP.core.DistanceToSingletonScore(
                IMP.core.HarmonicUpperBound(rwall, kwall), origin), ps[1])
        IMP.core.XYZ(ps[1]).set_coordinates_are_optimized(True)
        sf = IMP.core.RestraintsScoringFunction([r.get_restraint(), wall])
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function(sf)
        mc.set_kt(1.0)
        mc.set_return_best(False)
        mc.add_mover(IMP.core.BallMover(
            m, ps[1], IMP.core.XYZ.get_xyz_keys()[:2], 60.))
        mc.optimize(2000)

        def U(d):
            u = self.expected(d, cutoff)
            if d > rwall:
                u += 0.5 * kwall * (d - rwall) ** 2
            return u
        edges = [0., 100., 125., 150., 175., 200., 250., 300., 400., 500.]
        dr = 0.25
        weights = [0.] * (len(edges) - 1)
        d = dr / 2
        while d < 600.:
            for k in range(len(edges) - 1):
                if edges[k] <= d < edges[k + 1]:
                    weights[k] += d * math.exp(-U(d)) * dr
            d += dr
        norm = sum(weights)
        pexp = [w / norm for w in weights]

        counts = [0] * (len(edges) - 1)
        nsamp = 0
        xyz = IMP.core.XYZ(ps[1])
        for i in range(4000):
            mc.optimize(50)
            d = xyz.get_coordinates().get_magnitude()
            for k in range(len(edges) - 1):
                if edges[k] <= d < edges[k + 1]:
                    counts[k] += 1
            nsamp += 1
        for k in range(len(pexp)):
            self.assertAlmostEqual(counts[k] / nsamp, pexp[k], delta=0.025,
                                   msg="bin %g-%g" % (edges[k],
                                                      edges[k + 1]))

    def test_real_table(self):
        """A real table (in kT), if given, scores every grid point as tabulated

        Set IMP_PMI_TEST_PMF_TABLE to a table in kT whose last row is the
        cutoff, e.g. the AMPAR table from the cryo-ET pipeline."""
        fname = os.environ.get('IMP_PMI_TEST_PMF_TABLE')
        if not fname or not os.path.exists(fname):
            self.skipTest("set IMP_PMI_TEST_PMF_TABLE to test a real table")
        dist, vals = PMFRestraint.read_table(fname)
        cutoff = dist[-1]
        m = IMP.Model()
        ps = make_particles(m, [(0, 0, 0), (cutoff, 0, 0)])
        r = PMFRestraint(ps, fname, cutoff, 'kT')
        sf = IMP.core.RestraintsScoringFunction([r.get_restraint()])
        xyz = IMP.core.XYZ(ps[1])
        for d, w in list(zip(dist, vals))[::7]:
            xyz.set_coordinates(IMP.algebra.Vector3D(d, 0, 0))
            self.assertAlmostEqual(sf.evaluate(False), w - vals[-1],
                                   delta=1e-6)
        self.assertAlmostEqual(r.get_pmf(cutoff), 0., delta=1e-8)


if __name__ == '__main__':
    IMP.test.main()
