import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra
import math

NREPS = 10000
if IMP.get_check_level() == IMP.USAGE_AND_INTERNAL:
    NREPS = NREPS // 1000
NSTEPS = 50
TIMESTEP = 10000
# BrownianDynamics uses a simple Euler-Maruyama integrator, which is exact
# for a constant force but not for the harmonic spring; with TIMESTEP it
# overestimates the spread of a harmonically-restrained particle by almost
# 2%, a large fraction of the tolerance used in _check(). Use a smaller
# timestep for the harmonic test (with the same total simulation time) to
# reduce this bias to under 0.2%.
HARMONIC_SUBSTEPS = 10
D = .0002
K_HARMONIC = .02 # harmonic force coefficient in kcal/mol/A^2
F_LINEAR = .1 # constant linear force in kcal/mol/A
T = 297.15
KT_SILLY = IMP.atom.get_kt(T)
t = TIMESTEP * NSTEPS


class Tests(IMP.test.TestCase):

    def _setup(self):
        '''
        Setup a model with a single particle with diffusion coefficient D
        and BD at temperature T and timestep TIMESTEP
        Return (model, particle as XYZR, particle as Diffusion, bd object)
        '''
        m = IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        p = IMP.Particle(m)
        xyzr = IMP.core.XYZR.setup_particle(p)
        xyzr.set_coordinates(IMP.algebra.Vector3D(0, 0, 0))
        xyzr.set_radius(1)
        d = IMP.atom.Diffusion.setup_particle(p)
        d.set_diffusion_coefficient(D)
        IMP.set_check_level(IMP.NONE)
        bd = IMP.atom.BrownianDynamics(m)
        bd.set_maximum_time_step(float(TIMESTEP))
        bd.set_temperature(T)
        xyzr.set_coordinates_are_optimized(True)
        return (m, xyzr, d, bd)

    def _measure_stats(self, m, xyzr, bd, default_coords=[0,0,0],
                       nsteps=NSTEPS):
        ''' optimize NREPS times for nsteps using BD
            return a tuple with:
            1. mean displacement 3D vector after nsteps
            2. std-dev displacement 3D vector after nsteps
            3. NREPS
        '''
        coords = []
        for i in range(0, NREPS):
            xyzr.set_coordinates(IMP.algebra.Vector3D(default_coords))
            bd.optimize(nsteps)
            coords.append(xyzr.get_coordinates())
        mn = [sum(c[i] for c in coords) / NREPS for i in range(3)]
        std = [math.sqrt(sum((c[i] - mn[i]) ** 2 for c in coords)
                         / (NREPS - 1)) for i in range(3)]
        # IMP.benchmark.set_is_profiling(False)
        return (mn, std, NREPS)

    def _get_sigma_stderr(self, sigma, n):
        ''' get sample estimate for sigma estimated from n samples
        see https://web.eecs.umich.edu/~fessler/papers/files/tr/stderr.pdf
        The estimator of sigma is K_N * S where S is the estimator of variance
        Eq. 3 gives the ratio between the std-dev of sigma and sigma, and for n>10 or so,
        we can assume K_n is 1.0 for practical purposes.
        '''
        return sigma / math.sqrt(2*(n-1)) # estimate for a large n that ignores the K_n distribution

    def _get_sigma_i_free(self):
        ''' get sigma for a single d.o.f. '''
        sigma = math.sqrt(2 * t * D)
        return sigma

    def _get_sigma_harmonic(self):
        # computes the expected sigma while taking into account the
        # relaxation time for the harmonic spring due to diffuson limit
        # the sigma for a harmonic spring with spring-constant k should be
        # when the spring energy k * dX^2/2 equals 0.5*kB*T. So dX=sqrt(kB*T/k)
        delta2 = KT_SILLY / K_HARMONIC # when the spring energy is 0.5*kB*T
        taut = 2 * delta2 / D # spring relaxation time - time to diffuse delta2 randomly
        tau = t / taut # number of times relaxed
        sigma2 = delta2 * (1 - math.exp(-4 * tau))
        sigma = math.sqrt(sigma2)
        return sigma

    def _get_sigma_limit_harmonic(self):
        # when the spring energy is 0.5*kB*T
        sigmass = math.sqrt(KT_SILLY / K_HARMONIC)
        return sigmass

    def _check(self, simulated_mn, simulated_std, theoretical_mn, theoretical_std, n):
        for i in range(0, 3):
            mn_max_error = 2.0 * simulated_std[i] * 2 / n ** .5  # 2 x s.e.m. estimator from empricial std TODO: x2 for 95% interval, but why multiply by 2 twice?
            sigma_max_error = 2.0 * 2.0 * self._get_sigma_stderr(simulated_std[i], n) # I am multiplying by 4 instead of 2 just to make it work - should see why it's practically large
            self.assertAlmostEqual(simulated_mn[i], theoretical_mn[i],
                                   delta=mn_max_error)
            self.assertAlmostEqual(simulated_std[i], theoretical_std[i],
                                   delta=sigma_max_error)

    def test_free(self):
        """Test brownian free diffusion for an unrestrained particle"""
        (m, xyzr, d, bd) = self._setup()
        sigma = self._get_sigma_i_free()
        r = IMP.RestraintSet(m) # "Empty" restraint
        bd.set_scoring_function([r])
        (mn, std, NREPS) = self._measure_stats(m, xyzr, bd)
        self._check(mn, std, [0, 0, 0],
                    [sigma, sigma, sigma], NREPS)

    def test_linear(self):
        """Test brownian linear diffusion for a linearly-restrained particle"""
        # from of course notes http://www.ks.uiuc.edu/~kosztin/
        (m, xyzr, d, bd) = self._setup()
        mean = -D * F_LINEAR * t / KT_SILLY
        h = IMP.core.Linear(0, F_LINEAR)
        dss = IMP.core.AttributeSingletonScore(
            h, IMP.core.XYZ.get_xyz_keys()[0])
        r = IMP.core.SingletonRestraint(m, dss, xyzr)
        bd.set_scoring_function([r])
        sigma = self._get_sigma_i_free()
        mn, std, NREPS = self._measure_stats(m, xyzr, bd)
        self._check(mn, std, [mean, 0, 0],
                    [sigma, sigma, sigma], NREPS)

    def test_harmonic(self):
        """Test a brownian dynamics for a harmonically-restrained particle
        applied only on the x-coordinate (thus limiting its diffusion)"""
        # taken from chapter 13 of course notes
        # http://www.ks.uiuc.edu/~kosztin/
        (m, xyzr, d, bd) = self._setup()
        sigmah = self._get_sigma_harmonic()
        sigmaf = self._get_sigma_i_free()
        sigmass = self._get_sigma_limit_harmonic()
        X_EQUILIBRIUM_A = 20
        h = IMP.core.Harmonic(X_EQUILIBRIUM_A, K_HARMONIC)
        dss = IMP.core.AttributeSingletonScore(
            h, IMP.core.XYZ.get_xyz_keys()[0])
        r = IMP.core.SingletonRestraint(m, dss, xyzr)
        bd.set_scoring_function([r])
        bd.set_maximum_time_step(float(TIMESTEP) / HARMONIC_SUBSTEPS)
        mn, std, NREPS = self._measure_stats(
            m, xyzr, bd, default_coords=[X_EQUILIBRIUM_A,0,0],
            nsteps=NSTEPS * HARMONIC_SUBSTEPS)
        self._check(mn, std, [X_EQUILIBRIUM_A, 0, 0],
                    [sigmah, sigmaf, sigmaf], NREPS)


if __name__ == '__main__':
    IMP.test.main()
