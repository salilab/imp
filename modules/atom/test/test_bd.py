import IMP
import IMP.test
import IMP.core
import IMP.atom
import sympy
from sympy.physics.units import *
import math
nreps=10000
nsteps=500
timestep=100
timestep_u=timestep*femto*second
t_u= timestep_u*nsteps
angstrom= sympy.Rational(1,10)*nano*meter
D=.0002
D_u= .0002*angstrom*angstrom/(femto*second)
k=.01
k_u=  IMP.atom.get_spring_constant_in_femto_newtons_per_angstrom(k)*femto*newton/angstrom
f=.1
f_u= IMP.atom.get_force_in_femto_newtons(f)*femto*newton
kt_silly=IMP.atom.get_kb_t(273.)
kT_u= IMP.atom.get_energy_in_femto_joules(kt_silly)*femto*joule

class BDTests(IMP.test.TestCase):
    def _setup(self):
        m= IMP.Model()
        IMP.set_log_level(IMP.SILENT)
        p= IMP.Particle(m)
        xyzr= IMP.core.XYZR.setup_particle(p)
        xyzr.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        xyzr.set_radius(1)
        d=IMP.atom.Diffusion.setup_particle(p)
        d.set_d(D)
        bd= IMP.atom.BrownianDynamics(m)
        bd.set_maximum_time_step(float(timestep))
        xyzr.set_coordinates_are_optimized(True)
        return (m, xyzr, d, bd)
    def _measure(self, m, xyzr, bd):
        ub= IMP.algebra.Vector3D(50,50,50)
        h = IMP.statistics.Histogram3D(1, IMP.algebra.BoundingBox3D(-ub, ub))
        for i in range(0,nreps):
            xyzr.set_coordinates(IMP.algebra.Vector3D(0,0,0))
            bd.optimize(nsteps)
            h.add(xyzr.get_coordinates())
            if i%1000==0:
                print i, xyzr.get_coordinates()
        print "computing"
        mn= h.get_mean()
        std= h.get_standard_deviation(mn)
        print mn, std
        return (mn, std)
    #def _measure_x(self, m, xyzr, bd):
    #    ub= IMP.algebra.Vector1D(50)
    #    h = IMP.statistics.Histogram1D(.1, IMP.algebra.BoundingBox1D(-ub, ub))
    #   for i in range(0,nreps):
    #        xyzr.set_coordinates(IMP.algebra.Vector3D(0,0,0))
    #        bd.optimize(nsteps)
    #        h.add(IMP.algebra.Vector1D(xyzr.get_coordinates()[0]))
    #        if i%1000==0:
    #            print i, xyzr.get_coordinates()
    #    mn= h.get_mean()
    #    std= h.get_standard_deviation(mn)
    ##    print mn, std
    ##   return mn, std
    def _get_sigma_free(self):
        sigma= (6.0*t_u*D_u)**sympy.Rational(1,2)
        return sigma
    def _get_sigma_1_free(self):
        sigma= (2.0*t_u*D_u)**sympy.Rational(1,2)
        return sigma
    def _get_sigma_harmonic(self):
        delta2= (kT_u/k_u)
        delta= delta2**sympy.Rational(1,2)
        print "delta", delta
        taut= 2*delta2/D_u
        print "taut", taut.evalf()
        tau= t_u/taut
        print "tau", tau.evalf()
        scale= 2*delta2
        print "scale", scale.evalf()
        sigma2= scale*(1-sympy.E**(-4*tau))
        print "sigma2", sigma2.evalf()
        sigma= sigma2**sympy.Rational(1,2)
        print "sigma", sigma.evalf()
        return sigma
    def _get_sigma_limit_harmonic(self):
        sigmass= 2*kT_u/k_u
        print "sigma steady state 2", sigmass.evalf()
        return sigmass**sympy.Rational(1,2)
    def _check(self, (mn, std), (calc_mn, calc_std)):
        print [(x.evalf()/angstrom).evalf() for x in calc_std]
        print "mean", mn, [float((x/angstrom).evalf()) for x in calc_mn]
        print "std", std, [float((x/angstrom).evalf()) for x in calc_std]
        for i in range(0,3):
            self.assertAlmostEqual(mn[i], float((calc_mn[i]/angstrom).evalf()), delta=2)
            self.assertAlmostEqual(std[i], float((calc_std[i]/angstrom).evalf()), delta=2)
    def test_free(self):
        """Test a brownian free diffusion"""
        (m, xyzr, d, bd)= self._setup()
        (mn, std)= self._measure(m, xyzr, bd)
        print mn, std
        sigma= self._get_sigma_1_free()
        self._check((mn, std), ([0*angstrom, 0*angstrom, 0*angstrom], [sigma, sigma, sigma]))
    def test_linear(self):
        """Test a brownian linear"""
        # from of course notes http://www.ks.uiuc.edu/~kosztin/
        (m, xyzr, d, bd)= self._setup()
        #print D_u, f_u, kT_u
        #print D_u*f_u, f_u/kT_u, 1/kT_u
        mean= -D_u*f_u*t_u/kT_u
        print "mean", mean
        h = IMP.core.Linear(0, f)
        dss= IMP.core.AttributeSingletonScore(h, IMP.core.XYZ.get_xyz_keys()[0])
        r= IMP.core.SingletonRestraint(dss, xyzr)
        m.add_restraint(r)
        sigma= self._get_sigma_1_free()
        mn, std= self._measure(m, xyzr, bd)
        self._check((mn, std), ([mean, 0*angstrom, 0*angstrom], [sigma, sigma, sigma]))
    def test_harmonic(self):
        """Test a brownian harmonic"""
        # taken from chapter 13 of course notes http://www.ks.uiuc.edu/~kosztin/
        (m, xyzr, d, bd)= self._setup()
        sigma=self._get_sigma_harmonic()
        sigmaf= self._get_sigma_1_free()
        sigmass= self._get_sigma_limit_harmonic()

        h = IMP.core.Harmonic(0, k)
        dss= IMP.core.AttributeSingletonScore(h, IMP.core.XYZ.get_xyz_keys()[0])
        r= IMP.core.SingletonRestraint(dss, xyzr)
        m.add_restraint(r)
        mn, std= self._measure(m, xyzr, bd)
        self._check((mn, std), ([0*angstrom, 0*angstrom, 0*angstrom], [sigma, sigmaf, sigmaf]))
if __name__ == '__main__':
    IMP.test.main()
