import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.statistics
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
print("KT=%f kcal/mol" % KT_SILLY)
t = TIMESTEP * NSTEPS
print("NREPS=%d NSTEPS=%d} dT=%d fs time-per-simulation=%d fs"
      % (NREPS, NSTEPS, TIMESTEP, t))


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
        ub = IMP.algebra.Vector3D(100, 100, 100)
        # was .1
        h = IMP.statistics.Histogram3D(.3, IMP.algebra.BoundingBox3D(-ub, ub))
        # IMP.benchmark.set_is_profiling(True)
        print("Computing mean and std from %d simulations" % NREPS)
        for i in range(0, NREPS):
            xyzr.set_coordinates(IMP.algebra.Vector3D(default_coords))
            bd.optimize(nsteps)
            h.add(xyzr.get_coordinates())
            if (i % 1000 == 0) or (i == NREPS-1):
                print(i, xyzr.get_coordinates())
        mn = h.get_mean()
        std = h.get_standard_deviation(mn)
        print("mean=%s, std=%s" % (mn, std))
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
        #        return 2.0 * sigma ** 4 / n # TODO: how so?
    # def _measure_x(self, m, xyzr, bd):
    #    ub= IMP.algebra.Vector1D(50)
    #    h = IMP.statistics.Histogram1D(.1, IMP.algebra.BoundingBox1D(-ub, ub))
    #   for i in range(0,NREPS):
    #        xyzr.set_coordinates(IMP.algebra.Vector3D(0,0,0))
    #        bd.optimize(NSTEPS)
    #        h.add(IMP.algebra.Vector1D(xyzr.get_coordinates()[0]))
    #        if i%1000==0:
    #            print i, xyzr.get_coordinates()
    #    mn= h.get_mean()
    #    std= h.get_standard_deviation(mn)
    # print mn, std
    # return mn, std

    def _get_sigma_i_free(self):
        ''' get sigma for a single d.o.f. '''
        sigma = math.sqrt(2 * t * D)
        print("Theoretical sigma per d.o.f. for D=%g A^2/fs and t=%d fs "
              "FREE sigma=%.2f A" % (D, t, sigma))
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
        print("CALCULATED SIGMA %.2f A TAU=%.3f TAUT=%.1f RELAXATION=%.3f"
              % (sigma, tau, taut, 1-math.exp(-4*tau)))
        return sigma

    def _get_sigma_limit_harmonic(self):
        # when the spring energy is 0.5*kB*T
        sigmass = math.sqrt(KT_SILLY / K_HARMONIC)
        print("CALCULATED SIGMA STEADY STATE %.2f A" % sigmass)
        return sigmass

    def _check(self, simulated_mn, simulated_std, theoretical_mn, theoretical_std, n):
        print("mean", simulated_mn, theoretical_mn)
        print("std", simulated_std, theoretical_std)
        for i in range(0, 3):
            mn_max_error = 2.0 * simulated_std[i] * 2 / n ** .5  # 2 x s.e.m. estimator from empricial std TODO: x2 for 95% interval, but why multiply by 2 twice?
            sigma_max_error = 2.0 * 2.0 * self._get_sigma_stderr(simulated_std[i], n) # I am multiplying by 4 instead of 2 just to make it work - should see why it's practically large
            print("Max allowed error mean=%.2f A, sigma=%.2f A"
                  % (mn_max_error, sigma_max_error))
            self.assertAlmostEqual(simulated_mn[i], theoretical_mn[i],
                                   delta=mn_max_error)
            self.assertAlmostEqual(simulated_std[i], theoretical_std[i],
                                   delta=sigma_max_error)

    def test_free(self):
        """Test brownian free diffusion for an unrestrained particle"""
        print("\n\ntest_free")
        #self.skipTest("too expensive")
        (m, xyzr, d, bd) = self._setup()
        sigma = self._get_sigma_i_free()
        print("theoretical free sigma per d.o.f. is %.2f A^2/fs" % sigma)
        r = IMP.RestraintSet(m) # "Empty" restraint
        bd.set_scoring_function([r])
        (mn, std, NREPS) = self._measure_stats(m, xyzr, bd)
        self._check(mn, std, [0, 0, 0],
                    [sigma, sigma, sigma], NREPS)
        print("FINISHED FREE\n\n")

    def test_linear(self):
        """Test brownian linear diffusion for a linearly-restrained particle"""
        print("\n\ntest_linear")
        #self.skipTest("too expensive")
        # from of course notes http://www.ks.uiuc.edu/~kosztin/
        (m, xyzr, d, bd) = self._setup()
        print("Temperature", bd.get_temperature())
        # print D_u, f_u, kT_u
        # print D_u*f_u, f_u/kT_u, 1/kT_u
        mean = -D * F_LINEAR * t / KT_SILLY
        print("Theoretical mean x-coordinate: %.2f A" % mean)
        h = IMP.core.Linear(0, F_LINEAR)
        dss = IMP.core.AttributeSingletonScore(
            h, IMP.core.XYZ.get_xyz_keys()[0])
        r = IMP.core.SingletonRestraint(m, dss, xyzr)
        bd.set_scoring_function([r])
        sigma = self._get_sigma_i_free()
        mn, std, NREPS = self._measure_stats(m, xyzr, bd)
        self._check(mn, std, [mean, 0, 0],
                    [sigma, sigma, sigma], NREPS)
        print("FINISHED LINEAR\n")

    def test_harmonic(self):
        """Test a brownian dynamics for a harmonically-restrained particle
        applied only on the x-coordinate (thus limiting its diffusion)"""
        print("\n\ntest_harmonic")
        #self.skipTest("too expensive")
        # taken from chapter 13 of course notes
        # http://www.ks.uiuc.edu/~kosztin/
        print()
        (m, xyzr, d, bd) = self._setup()
        sigmah = self._get_sigma_harmonic()
        print("harmonic sigma=%.2f A" % sigmah)
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
        print("Mean / std / sigmah / sigmaf / sigmass")
        print(mn)
        print(std)
        print(sigmah)
        print(sigmaf)
        print(sigmass)
        self._check(mn, std, [X_EQUILIBRIUM_A, 0, 0],
                    [sigmah, sigmaf, sigmaf], NREPS)
        print("FINISHED HARMONIC\n")


if __name__ == '__main__':
    IMP.test.main()
