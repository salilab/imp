#!/usr/bin/env python

#standard imports
import random
from math import *

#IMP
import IMP
import IMP.core
import IMP.saxs
import IMP.atom
from IMP.isd import Scale,SAXSRestraint_LN

#unit testing framework
import IMP.test

class SAXSRestraint_LN_Test(IMP.test.TestCase):
    """test the ISD SAXS empirical Lognormal Restraint"""

    def setUp(self):
        #general stuff
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        #setup scales, DA and protein
        self.alpha = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.gamma = Scale.setup_particle(IMP.Particle(self.m), 1.0)
        self.DA = IMP.DerivativeAccumulator()
        self.prot = IMP.atom.read_pdb(self.get_input_file_name('6lyz.pdb'),
                self.m, IMP.atom.NonWaterNonHydrogenPDBSelector(), True, True)
        #experimental profile
        exp_profile = IMP.saxs.Profile(self.get_input_file_name('lyzexp.dat'))
        self.exp_profile = exp_profile
        self.qmin = exp_profile.get_min_q()
        self.qmax = exp_profile.get_max_q()
        self.dq = exp_profile.get_delta_q()
        #ISD SAXS restraint
        self.particles = IMP.atom.get_by_type(self.prot, IMP.atom.ATOM_TYPE)
        particles=self.particles
        self.sr = IMP.isd.SAXSRestraint_LN(particles, self.alpha, self.gamma, exp_profile)
        self.m.add_restraint(self.sr)

    def perturb_particles(self):
        "perturb particle positions with a gaussian noise of sd 1 angstrom"
        for p in self.particles:
            d=IMP.core.XYZ(p)
            d.set_coordinates([i+random.gauss(0,1) for i in d.get_coordinates()])

    def compute_prob(self):
        "compute likelihood of current model"
        model_profile = IMP.saxs.Profile(self.qmin, self.qmax, self.dq)
        model_profile.calculate_profile(self.particles)
        expected = 1.0
        alpha=self.alpha.get_scale()
        gamma=self.gamma.get_scale()
        for i in xrange(self.exp_profile.size()):
            iexp=self.exp_profile.get_intensity(i)
            icalc=model_profile.get_intensity(i)
            err=self.exp_profile.get_error(i)/iexp
            expected *= \
                1/(sqrt(2*pi)*iexp*alpha*err)*exp(-1/(2*(alpha*err)**2)*log(iexp/(gamma*icalc))**2)
        return expected

    def compute_ene(self):
        "compute log-likelihood of current model"
        model_profile = IMP.saxs.Profile(self.qmin, self.qmax, self.dq)
        model_profile.calculate_profile(self.particles)
        expected = 0.0
        alpha=self.alpha.get_scale()
        gamma=self.gamma.get_scale()
        for i in xrange(self.exp_profile.size()):
            iexp=self.exp_profile.get_intensity(i)
            icalc=model_profile.get_intensity(i)
            err=self.exp_profile.get_error(i)/iexp
            expected += \
                log(sqrt(2*pi)*iexp*alpha*err)+1/(2*(alpha*err)**2)*log(iexp/(gamma*icalc))**2
        return expected

    def testSanityPE(self):
        "test if prob is exp(-score)"
        for i in xrange(10):
            no=random.uniform(0.1,100)
            self.alpha.set_scale(no)
            p=self.sr.get_probability()
            if p==0:
                continue
            p=-log(p)
            e=self.sr.evaluate(None)
            if e != 0:
                self.assertAlmostEqual(p/e,1)
            else:
                self.assertEqual(p,0)

    def testParticles(self):
        "test get_input_particles"
        expected = self.particles + [self.alpha.get_particle(),
                self.gamma.get_particle()]
        for i in self.sr.get_input_particles():
            self.assertTrue(i in expected)
            expected.remove(i)
        self.assertEqual(expected,[])

    def testContainers(self):
        "test get_input_containers"
        self.assertEqual(self.sr.get_input_containers(),[])

    def testValuePProt(self):
        """tests the probability by shuffling the atom's coordinates"""
        for i in xrange(10):
            self.perturb_particles()
            expected=self.compute_prob()
            observed =self.sr.get_probability()
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1)
            else:
                self.assertEqual(observed,0)

    def testValuePGamma(self):
        """tests the probability by shuffling gamma"""
        for i in xrange(10):
            self.gamma.set_scale(float(i)*9+1)
            expected=self.compute_prob()
            observed =self.sr.get_probability()
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1)
            else:
                self.assertEqual(observed,0)

    def testValuePAlpha(self):
        """tests the probability by shuffling alpha"""
        for i in xrange(10):
            self.alpha.set_scale(float(i)*9+1)
            expected=self.compute_prob()
            observed =self.sr.get_probability()
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1)
            else:
                self.assertEqual(observed,0)

    def testValueEProt(self):
        """tests the energy by shuffling the atom's coordinates"""
        for i in xrange(10):
            self.perturb_particles()
            expected=self.compute_ene()
            observed =self.sr.evaluate(None)
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1,delta=0.001)
            else:
                self.assertEqual(observed,0)

    def testValueEGamma(self):
        """tests the energy by shuffling gamma"""
        for i in xrange(10):
            self.gamma.set_scale(float(i)*9+1)
            expected=self.compute_ene()
            observed =self.sr.evaluate(None)
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1,delta=0.001)
            else:
                self.assertEqual(observed,0)

    def testValueEAlpha(self):
        """tests the energy by shuffling alpha"""
        for i in xrange(10):
            self.alpha.set_scale(float(i)*9+1)
            expected=self.compute_ene()
            observed =self.sr.evaluate(None)
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1,delta=0.001)
            else:
                self.assertEqual(observed,0)



    



if __name__ == '__main__':
    IMP.test.main()
