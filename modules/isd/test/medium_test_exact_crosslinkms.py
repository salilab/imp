import IMP
import IMP.core
import IMP.algebra
import IMP.isd
import IMP.test
import pickle
import random
import functools
from math import pi, log, exp
import numpy as np
try:
    import jax
    import jax.numpy as jnp
    import IMP.jax
except ImportError:
    jax = None


def setupnuisance(m, initialvalue, minvalue, maxvalue, isoptimized=True):
    nuisance = IMP.isd.Scale.setup_particle(IMP.Particle(m), initialvalue)
    if minvalue:
        nuisance.set_lower(minvalue)
    if maxvalue:
        nuisance.set_upper(maxvalue)
    nuisance.set_is_optimized(nuisance.get_nuisance_key(), isoptimized)
    return nuisance


def lens_volume(a, b, r):
    """Volume of the intersection of balls of radii a and b, r apart"""
    rmin = np.minimum(a, b)
    rs = np.where(r > 0., r, 1.)
    mid = (pi * (a + b - r) ** 2
           * (r * r + 2. * r * (a + b) - 3. * (a - b) ** 2) / (12. * rs))
    return np.where(r >= a + b, 0.,
                    np.where(r <= abs(a - b), 4. / 3. * pi * rmin ** 3, mid))


def sphere_area_within(r, d, radius):
    """Area of the sphere of radius r, centered at the origin, that lies
       inside the ball of the given radius centered at distance d"""
    mid = pi * r * (radius * radius - (r - d) ** 2) / d
    return np.where(r + d <= radius, 4. * pi * r * r,
                    np.where((r <= d - radius) | (r >= d + radius),
                             0., mid))


@functools.lru_cache(maxsize=None)
def forward_model(dist, sigmai, sigmaj, length):
    """Probability that two points, uniform in spheres of radii sigmai and
       sigmaj whose centers are dist apart, are closer than length.

       This integrates over the distance s of the first point from the
       center of the second sphere: s has density
       sphere_area_within(s, dist, sigmai) / Vi, and the probability that
       the second point is within length of the first is
       lens_volume(sigmaj, length, s) / Vj. (The C++ code integrates over
       the distance between the two points instead.)"""
    smax = dist + sigmai
    points = sorted(set([0., smax] + [x for x in (abs(dist - sigmai),
                                                  abs(sigmaj - length),
                                                  sigmaj + length)
                                      if 0. < x < smax]))
    total = 0.
    for lo, hi in zip(points[:-1], points[1:]):
        # Composite Simpson's rule on each smooth piece
        s = np.linspace(lo, hi, 81)
        f = (sphere_area_within(s, dist, sigmai)
             * lens_volume(sigmaj, length, s))
        h = (hi - lo) / 80.
        total += h / 3. * (f[0] + f[-1] + 4. * f[1:-1:2].sum()
                           + 2. * f[2:-1:2].sum())
    voli = 4.0 / 3.0 * pi * sigmai ** 3
    volj = 4.0 / 3.0 * pi * sigmaj ** 3
    return total / voli / volj


class ReferenceRestraint:
    """Python implementation of ExactCrossLinkMSRestraint"""

    def __init__(self, length, slope):
        self.pairs = []
        self.length = length
        self.slope = slope

    def add_contribution(self, p1, p2, sigma1, sigma2, psi):
        self.pairs.append((p1, p2, sigma1, sigma2, psi))

    def get_probability(self):
        onemprob = 1.0
        for (p1, p2, sigma1, sigma2, psi) in self.pairs:
            if p1 != p2:
                dist = IMP.core.get_distance(IMP.core.XYZ(p1),
                                             IMP.core.XYZ(p2))
            else:
                dist = 36.0 / 35.0 * IMP.core.XYZR(p1).get_radius()
            dist = max(dist, 0.0001)
            psi = psi.get_scale()
            pofr = forward_model(dist, sigma1.get_scale(),
                                 sigma2.get_scale(), self.length)
            onemprob *= (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi))
                         * exp(-self.slope * dist))
        return 1.0 - onemprob


def setup_system(cls=IMP.isd.ExactCrossLinkMSRestraint):
    m = IMP.Model()
    p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (0, 5, 0))
    p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (5, 0, 0))
    p3 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (0, 0, 15))
    sigma1 = setupnuisance(m, 5, 0, 100, False)
    sigma2 = setupnuisance(m, 6, 0, 100, False)
    psi1 = setupnuisance(m, 0.1, 0.0, 0.5, False)
    psi2 = setupnuisance(m, 0.2, 0.0, 0.5, False)

    dr1 = cls(m, 10., 0.01)
    # Weights should be ignored
    dr1.set_weight(9999.0)
    dr1.add_contribution((p1, p2), (sigma1, sigma2), psi1)
    dr1.add_contribution((p2, p3), (sigma1, sigma2), psi2)

    dr2 = cls(m, 10., 0.01)
    dr2.add_contribution((p1, p3), (sigma1, sigma2), psi1)
    lw = IMP.isd.LogWrapper([dr1, dr2], 1.0)
    return m, p1, p2, p3, sigma1, sigma2, psi1, psi2, lw


def make_test_restraint():
    m = IMP.Model()
    p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
    p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
    sigma1 = setupnuisance(m, 5, 0, 100, False)
    sigma2 = setupnuisance(m, 5, 0, 100, False)
    psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
    dr = IMP.isd.ExactCrossLinkMSRestraint(m, 10., 0.01)
    dr.set_weight(0.10)
    dr.set_name("test restraint")
    dr.add_contribution((p1, p2), (sigma1, sigma2), psi)
    return m, p1, p2, sigma1, sigma2, psi, dr


def numerical_deriv(r, get, set, h=1e-5):
    orig = get()
    set(orig + h)
    up = r.evaluate(False)
    set(orig - h)
    down = r.evaluate(False)
    set(orig)
    return (up - down) / (2. * h)


class Tests(IMP.test.TestCase):

    def test_api(self):
        """Test that the API matches that of CrossLinkMSRestraint"""
        def get_api(cls):
            return frozenset(x for x in dir(cls) if not x.startswith('_'))
        self.assertEqual(get_api(IMP.isd.ExactCrossLinkMSRestraint),
                         get_api(IMP.isd.CrossLinkMSRestraint))

    def test_default(self):
        """Test default-constructed restraint"""
        dr = IMP.isd.ExactCrossLinkMSRestraint()
        self.assertRaisesUsageException(dr.evaluate, False)

    def test_accessors(self):
        """Test ExactCrossLinkMSRestraint accessors"""
        m, p1, p2, sigma1, sigma2, psi, dr = make_test_restraint()
        self.assertEqual(dr.get_number_of_contributions(), 1)
        self.assertEqual(dr.get_contribution_particle_indexes(0),
                         (p1.get_particle_index(), p2.get_particle_index()))
        self.assertEqual(dr.get_contribution_psi_index(0),
                         psi.get_particle_index())
        self.assertEqual(dr.get_contribution_sigma_indexes(0),
                         (sigma1.get_particle_index(),
                          sigma2.get_particle_index()))
        self.assertAlmostEqual(dr.get_slope(), 0.01, delta=1e-4)
        self.assertAlmostEqual(dr.get_length(), 10.0, delta=1e-4)
        self.assertTrue(dr.get_has_slope())
        self.assertFalse(dr.get_is_length_variable())
        self.assertFalse(dr.get_log_prob())
        length = setupnuisance(m, 21., 0, 100, False)
        dr = IMP.isd.ExactCrossLinkMSRestraint(m, length, True)
        self.assertAlmostEqual(dr.get_length(), 21.0, delta=1e-4)
        self.assertTrue(dr.get_is_length_variable())
        self.assertFalse(dr.get_has_slope())
        self.assertTrue(dr.get_log_prob())

    def test_forward_model(self):
        """Test the forward model against the published definition"""
        m = IMP.Model()
        p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        sigma1 = setupnuisance(m, 1., 0, 100, False)
        sigma2 = setupnuisance(m, 1., 0, 100, False)
        psi = setupnuisance(m, 0., 0., 0.5, False)

        def get_f(dist, s1, s2, length):
            # With psi = 0, no slope and one contribution, the restraint
            # returns the forward model itself
            r = IMP.isd.ExactCrossLinkMSRestraint(m, length)
            r.add_contribution((p1, p2), (sigma1, sigma2), psi)
            p2.set_coordinates(IMP.algebra.Vector3D(dist, 0., 0.))
            sigma1.set_scale(s1)
            sigma2.set_scale(s2)
            return r.get_probability()

        # Spheres entirely within, or entirely beyond, the linker length
        self.assertAlmostEqual(get_f(10., 2., 3., 15.5), 1., delta=1e-8)
        self.assertAlmostEqual(get_f(20.5, 2., 3., 15.), 0., delta=1e-8)
        # Concentric spheres of radius sigma: the distance between two
        # independent uniform points in a ball of radius sigma has the
        # distribution function x^3 - 9/16 x^4 + 1/32 x^6, with
        # x = length / sigma <= 2 (Deltheil 1926; Hammersley 1950,
        # Ann. Math. Statist. 21, 447)
        for sigma in (10.5, 12., 15., 21.):
            x = 21. / sigma
            self.assertAlmostEqual(get_f(0., sigma, sigma, 21.),
                                   x ** 3 - 9. / 16. * x ** 4 + x ** 6 / 32.,
                                   delta=1e-8)
        # Brute-force sampling of the definition
        rng = np.random.RandomState(42)

        def sample_ball(radius, n):
            v = rng.normal(size=(n, 3))
            v /= np.linalg.norm(v, axis=1)[:, None]
            return v * (radius * rng.uniform(size=n) ** (1. / 3.))[:, None]
        n = 200000
        for dist, s1, s2, length in [(21., 5., 5., 21.), (18., 12., 12., 21.),
                                     (24., 5., 5., 21.), (10., 3., 8., 27.),
                                     (30., 13.5, 13.5, 27.)]:
            x1 = sample_ball(s1, n)
            x2 = sample_ball(s2, n) + np.array([dist, 0., 0.])
            mc = np.mean(np.linalg.norm(x1 - x2, axis=1) < length)
            # Standard error is at most 0.0012
            self.assertAlmostEqual(get_f(dist, s1, s2, length), mc,
                                   delta=0.006)
            self.assertAlmostEqual(get_f(dist, s1, s2, length),
                                   forward_model(dist, s1, s2, length),
                                   delta=1e-6)

    def test_old_restraint_unchanged(self):
        """Test that CrossLinkMSRestraint still uses the old model"""
        m, p1, p2, p3, sigma1, sigma2, psi1, psi2, lw = setup_system(
            IMP.isd.CrossLinkMSRestraint)
        self.assertAlmostEqual(lw.evaluate(False), 2.48402, delta=1e-4)
        m, p1, p2, p3, sigma1, sigma2, psi1, psi2, lw = setup_system()
        ref = 0.
        for pairs in (((p1, p2, psi1), (p2, p3, psi2)), ((p1, p3, psi1),)):
            r = ReferenceRestraint(10., 0.01)
            for a, b, psi in pairs:
                r.add_contribution(a, b, sigma1, sigma2, psi)
            ref -= log(r.get_probability())
        self.assertAlmostEqual(lw.evaluate(False), ref, delta=1e-6)

    def test_score_multiple_restraints(self):
        """Test random restraints against the Python reference"""
        m = IMP.Model()
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(30, 30, 30))
        for n in range(30):
            length = random.uniform(1, 40)
            slope = random.uniform(0, 0.1)
            dr = IMP.isd.ExactCrossLinkMSRestraint(m, length, slope)
            dr_lp = IMP.isd.ExactCrossLinkMSRestraint(m, length, slope, True)
            testdr = ReferenceRestraint(length, slope)
            for a in range(random.randint(1, 4)):
                p1 = IMP.core.XYZR.setup_particle(
                    IMP.Particle(m), IMP.algebra.Sphere3D(
                        IMP.algebra.get_random_vector_in(bb),
                        random.uniform(1.0, 10.0)))
                s1 = setupnuisance(m, random.uniform(1.0, 11.0), 0, 100,
                                   False)
                if random.randint(0, 1):
                    p2, s2 = p1, s1
                else:
                    p2 = IMP.core.XYZR.setup_particle(
                        IMP.Particle(m), IMP.algebra.Sphere3D(
                            IMP.algebra.get_random_vector_in(bb),
                            random.uniform(1.0, 10.0)))
                    s2 = setupnuisance(m, random.uniform(1.0, 11.0), 0, 100,
                                       False)
                psi = setupnuisance(m, random.uniform(0.01, 0.49), 0.0, 0.5,
                                    False)
                dr.add_contribution((p1, p2), (s1, s2), psi)
                dr_lp.add_contribution((p1, p2), (s1, s2), psi)
                testdr.add_contribution(p1, p2, s1, s2, psi)
            prob = testdr.get_probability()
            self.assertAlmostEqual(dr.evaluate(False), prob, delta=1e-6)
            self.assertAlmostEqual(dr_lp.evaluate(False), -log(prob),
                                   delta=1e-5)

    def test_no_cpp_derivatives(self):
        """Test that C++ derivatives are not calculated"""
        m, p1, p2, sigma1, sigma2, psi, dr = make_test_restraint()
        p2.set_coordinates((12, 0, 0))
        dr.evaluate(True)
        for p in (p1, p2):
            self.assertLess(p.get_derivatives().get_magnitude(), 1e-12)
        for s in (sigma1, sigma2, psi):
            self.assertEqual(s.get_scale_derivative(), 0.)

    def test_inputs(self):
        """Test restraint inputs"""
        m = IMP.Model()
        p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m))
        sigma = setupnuisance(m, 5, 0, 100, False)
        psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
        length = setupnuisance(m, 21., 0, 100, False)
        r = IMP.isd.ExactCrossLinkMSRestraint(m, length)
        r.add_contribution((p1, p2), (sigma, sigma), psi)
        self.assertEqual(
            sorted(IMP.Particle.get_from(x).get_index()
                   for x in r.get_inputs()),
            sorted(x.get_particle_index()
                   for x in (p1, p2, sigma, psi, length)))

    def test_serialize(self):
        """Test (un-)serialize of ExactCrossLinkMSRestraint"""
        m, p1, p2, sigma1, sigma2, psi, dr = make_test_restraint()
        dr.set_source_residue1(1)
        dr.set_source_residue2(4)
        dr.set_source_protein1("r1")
        dr.set_source_protein2("r2")
        p2.set_coordinates((12, 0, 0))
        score = dr.evaluate(False)
        dump = pickle.dumps(dr)
        del dr

        newdr = pickle.loads(dump)
        self.assertIsInstance(newdr, IMP.isd.ExactCrossLinkMSRestraint)
        self.assertAlmostEqual(newdr.get_weight(), 0.10, 0.01)
        self.assertEqual(newdr.get_name(), "test restraint")
        self.assertEqual(newdr.get_number_of_contributions(), 1)
        self.assertAlmostEqual(newdr.get_slope(), 0.01, delta=1e-4)
        self.assertAlmostEqual(newdr.get_length(), 10.0, delta=1e-4)
        self.assertEqual(newdr.get_source_residue1(), 1)
        self.assertEqual(newdr.get_source_residue2(), 4)
        self.assertEqual(newdr.get_source_protein1(), "r1")
        self.assertEqual(newdr.get_source_protein2(), "r2")
        self.assertAlmostEqual(newdr.evaluate(False), score, delta=1e-8)

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of ExactCrossLinkMSRestraint via polymorphic
           pointer"""
        m, p1, p2, sigma1, sigma2, psi, dr = make_test_restraint()
        sf = IMP.core.RestraintsScoringFunction([dr])
        dump = pickle.dumps(sf)
        newsf = pickle.loads(dump)
        newdr, = newsf.restraints
        newdr = IMP.isd.ExactCrossLinkMSRestraint.get_from(newdr)
        self.assertEqual(newdr.get_name(), "test restraint")

    def test_static_info(self):
        """Test restraint static info"""
        m, p1, p2, sigma1, sigma2, psi, dr = make_test_restraint()
        self.assertIsNone(dr.get_static_info())
        dr.set_source_residue1(1)
        dr.set_source_residue2(4)
        dr.set_source_protein1("foo")
        dr.set_source_protein2("bar")
        s = dr.get_static_info()
        self.assertEqual(s.get_number_of_string(), 2)
        self.assertEqual(s.get_string_value(0), "foo")
        self.assertEqual(s.get_string_value(1), "bar")
        self.assertEqual(s.get_number_of_int(), 2)
        self.assertEqual(s.get_int_value(0), 1)
        self.assertEqual(s.get_int_value(1), 4)
        self.assertEqual(s.get_number_of_particle_indexes(), 3)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_score(self):
        """Test JAX score against C++"""
        for dist in (0., 3., 9., 14., 21., 25., 30., 40.):
            m = IMP.Model()
            p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (0, 0, 0))
            p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (dist, 0, 0))
            sigma1 = setupnuisance(m, 4., 0, 100, False)
            sigma2 = setupnuisance(m, 7., 0, 100, False)
            psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
            for lp in (False, True):
                r = IMP.isd.ExactCrossLinkMSRestraint(m, 21., 0.01, lp)
                r.add_contribution((p1, p2), (sigma1, sigma2), psi)
                self.assertAlmostEqual(float(jnp.sum(r._evaluate_jax())),
                                       r.evaluate(False), delta=1e-6)

    def check_jax_derivatives(self, r, xyzs, radii, scales):
        """Check JAX derivatives of r against finite differences of the
           C++ score. Nuisances must be the only Scales in the Model,
           and be given in particle setup order."""
        ji = r._get_jax(space=IMP.jax.FreeSpace)
        X = ji.get_jax_model()
        score, g = jax.jit(jax.value_and_grad(
            lambda X: jnp.sum(ji.score_func(X))))(X)
        self.assertAlmostEqual(float(score), r.evaluate(False), delta=1e-6)
        for i, xyz in enumerate(xyzs):
            for k in range(3):
                d = numerical_deriv(r, lambda: xyz.get_coordinate(k),
                                    lambda v: xyz.set_coordinate(k, v))
                self.assertAlmostEqual(float(g['xyz'][i][k]), d,
                                       delta=1e-5 * max(1., abs(d)))
        for i, xyzr in enumerate(radii):
            d = numerical_deriv(r, xyzr.get_radius, xyzr.set_radius)
            self.assertAlmostEqual(float(g['r'][i]), d,
                                   delta=1e-5 * max(1., abs(d)))
        for i, s in enumerate(scales):
            d = numerical_deriv(r, s.get_scale, s.set_scale)
            self.assertAlmostEqual(float(g['nuisance'].data[i]), d,
                                   delta=1e-5 * max(1., abs(d)))

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_derivatives(self):
        """Test JAX derivatives against finite differences"""
        for log_prob in (False, True):
            for dist in (3., 9., 14., 21., 25., 30.):
                m = IMP.Model()
                p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (0, 0, 0))
                p2 = IMP.core.XYZ.setup_particle(
                    IMP.Particle(m), (dist * 0.6, dist * 0.8, 0))
                p3 = IMP.core.XYZ.setup_particle(IMP.Particle(m), (2, 3, 4))
                sigma1 = setupnuisance(m, 4., 0, 100, False)
                sigma2 = setupnuisance(m, 7., 0, 100, False)
                psi1 = setupnuisance(m, 0.1, 0.0, 0.5, False)
                psi2 = setupnuisance(m, 0.3, 0.0, 0.5, False)
                r = IMP.isd.ExactCrossLinkMSRestraint(m, 21., 0.02, log_prob)
                r.add_contribution((p1, p2), (sigma1, sigma2), psi1)
                r.add_contribution((p2, p3), (sigma2, sigma2), psi2)
                self.check_jax_derivatives(r, [p1, p2, p3], [],
                                           [sigma1, sigma2, psi1, psi2])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_derivatives_same_particle(self):
        """Test JAX derivatives when both ends are the same particle"""
        m = IMP.Model()
        p1 = IMP.core.XYZR.setup_particle(
            IMP.Particle(m), IMP.algebra.Sphere3D((0, 0, 0), 8.))
        sigma1 = setupnuisance(m, 5., 0, 100, False)
        psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
        r = IMP.isd.ExactCrossLinkMSRestraint(m, 10., 0.01, True)
        r.add_contribution((p1, p1), (sigma1, sigma1), psi)
        self.check_jax_derivatives(r, [p1], [p1], [sigma1, psi])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_derivatives_multiple(self):
        """Test JAX derivatives of LogWrapper against finite differences"""
        m, p1, p2, p3, sigma1, sigma2, psi1, psi2, lw = setup_system()
        self.check_jax_derivatives(lw, [p1, p2, p3], [],
                                   [sigma1, sigma2, psi1, psi2])

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_derivatives_zero_contribution(self):
        """Test JAX derivatives when one contribution is certain"""
        m = IMP.Model()
        ps = [IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                          IMP.algebra.Vector3D(x, 0, 0))
              for x in (0, 1, 30, 50)]
        sigma = setupnuisance(m, 2, 0, 100, False)
        # With psi=0 and spheres entirely within the linker length,
        # the first contribution has probability exactly 1
        psi1 = setupnuisance(m, 0.0, 0.0, 0.5, False)
        psi2 = setupnuisance(m, 0.1, 0.0, 0.5, False)
        r = IMP.isd.ExactCrossLinkMSRestraint(m, 10.0, True)
        r.add_contribution((ps[0], ps[1]), (sigma, sigma), psi1)
        r.add_contribution((ps[2], ps[3]), (sigma, sigma), psi2)
        self.assertAlmostEqual(r.evaluate(False), 0.0, delta=1e-8)
        r_other = IMP.isd.ExactCrossLinkMSRestraint(m, 10.0, True)
        r_other.add_contribution((ps[2], ps[3]), (sigma, sigma), psi2)

        ji = r._get_jax(space=IMP.jax.FreeSpace)
        X = ji.get_jax_model()
        g = jax.grad(lambda X: jnp.sum(ji.score_func(X)))(X)
        for leaf in jax.tree_util.tree_leaves(g):
            self.assertTrue(bool(jnp.all(jnp.isfinite(leaf))),
                            "Non-finite derivative: %s" % leaf)
        # The first contribution's 1-p is psi1, so d(score)/d(psi1) is
        # the product of 1-p for all other contributions
        self.assertAlmostEqual(float(g['nuisance'].data[1]),
                               1.0 - r_other.get_probability(), delta=1e-6)
        # Score does not depend on anything else
        self.assertAlmostEqual(float(jnp.sum(jnp.abs(g['xyz']))), 0.0,
                               delta=1e-6)
        self.assertAlmostEqual(float(g['nuisance'].data[0]), 0.0, delta=1e-6)
        self.assertAlmostEqual(float(g['nuisance'].data[2]), 0.0, delta=1e-6)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_derivatives_finite(self):
        """Test JAX derivatives are finite in degenerate configurations"""
        def get_grad(c1, c2, same_particle, s1=5., s2=5.):
            m = IMP.Model()
            p1 = IMP.core.XYZR.setup_particle(
                IMP.Particle(m), IMP.algebra.Sphere3D(c1, 4.0))
            p2 = IMP.core.XYZR.setup_particle(
                IMP.Particle(m), IMP.algebra.Sphere3D(c2, 4.0))
            sigma1 = setupnuisance(m, s1, 0, 100, False)
            sigma2 = setupnuisance(m, s2, 0, 100, False)
            psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
            r = IMP.isd.ExactCrossLinkMSRestraint(m, 10.0, 0.01, True)
            r.add_contribution((p1, p1 if same_particle else p2),
                               (sigma1, sigma2), psi)
            ji = r._get_jax(space=IMP.jax.FreeSpace)
            X = ji.get_jax_model()
            score = ji.score_func(X)
            self.assertAlmostEqual(float(jnp.sum(score)),
                                   r.evaluate(False), delta=1e-6)
            g = jax.grad(lambda X: jnp.sum(ji.score_func(X)))(X)
            for leaf in jax.tree_util.tree_leaves(g):
                self.assertTrue(bool(jnp.all(jnp.isfinite(leaf))),
                                "Non-finite derivative: %s" % leaf)

        # Coincident particles (zero distance)
        get_grad((0, 0, 0), (0, 0, 0), False)
        # Both ends of the cross-link on the same particle
        get_grad((0, 0, 0), (10, 0, 0), True)
        # dist == length, so breakpoint |dist - length| is zero
        get_grad((0, 0, 0), (10, 0, 0), False)
        # Equal sigmas and dist + length == sigma1 + sigma2
        get_grad((0, 0, 0), (2, 0, 0), False, 6., 6.)
        # Regular configuration
        get_grad((0, 0, 0), (15, 0, 0), False, 4., 7.)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_multiple(self):
        """Test _get_jax_multiple with old and new restraints mixed"""
        m = IMP.Model()
        ps = [IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                          IMP.algebra.Vector3D(x, 0, 0))
              for x in (0, 12, 25, 31)]
        sigma = setupnuisance(m, 5, 0, 100, False)
        psi = setupnuisance(m, 0.1, 0.0, 0.5, False)
        rs = []
        for cls in (IMP.isd.ExactCrossLinkMSRestraint,
                    IMP.isd.CrossLinkMSRestraint):
            for length, slope in ((21., 0.01), (21., 0.01), (15., 0.02)):
                r = cls(m, length, slope, True)
                r.add_contribution((ps[0], ps[1]), (sigma, sigma), psi)
                r.add_contribution((ps[2], ps[3]), (sigma, sigma), psi)
                rs.append(r)
        # ExactCrossLinkMSRestraint should only handle its own restraints,
        # grouped by length and slope
        jis, ungrouped = IMP.isd.ExactCrossLinkMSRestraint._get_jax_multiple(
            rs, IMP.jax.FreeSpace)
        self.assertEqual(len(jis), 2)
        self.assertEqual(len(ungrouped), 3)
        self.assertTrue(all(type(r) is IMP.isd.CrossLinkMSRestraint
                            for r in ungrouped))
        rset = IMP.RestraintSet(rs, 1.0)
        self.assertAlmostEqual(float(jnp.sum(rset._evaluate_jax())),
                               rset.evaluate(False), delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
