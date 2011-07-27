#!/usr/bin/env python

#standard imports
import random
from math import *
from scipy.special import gammainc

#IMP
import IMP
import IMP.core
import IMP.saxs
import IMP.atom
from IMP.isd import Scale,SAXSRestraint_marginal_LN

#unit testing framework
import IMP.test

class SAXSRestraint_marginal_LN_Test(IMP.test.TestCase):
    """test the ISD SAXS empirical Normal restraint"""

    def setUp(self):
        #general stuff
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        #setup scales, DA and protein
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
        self.sr = IMP.isd.SAXSRestraint_marginal_LN(particles, exp_profile)
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
        wx = [(self.exp_profile.get_intensity(i)/self.exp_profile.get_error(i))**2
                for i in xrange(self.exp_profile.size())]
        gammahat=1.0
        W=sum(wx)
        for i in xrange(self.exp_profile.size()):
                gammahat *= (self.exp_profile.get_intensity(i)/
                        model_profile.get_intensity(i))**(wx[i]/W)
        s2W = 0.0
        for i in xrange(self.exp_profile.size()):
            iexp=self.exp_profile.get_intensity(i)
            icalc=model_profile.get_intensity(i)
            s2W += wx[i]*log(iexp/(icalc*gammahat))**2
        expected = s2W**(-(self.exp_profile.size()-1)/2.0)* \
            gammainc((self.exp_profile.size()-1)/2., s2W/2.)
        return expected

    def compute_ene(self):
        "compute log-likelihood of current model"
        model_profile = IMP.saxs.Profile(self.qmin, self.qmax, self.dq)
        model_profile.calculate_profile(self.particles)
        wx = [(self.exp_profile.get_intensity(i)/self.exp_profile.get_error(i))**2
                for i in xrange(self.exp_profile.size())]
        gammahat=1.0
        W=sum(wx)
        for i in xrange(self.exp_profile.size()):
                gammahat *= (self.exp_profile.get_intensity(i)/
                        model_profile.get_intensity(i))**(wx[i]/W)
        s2W = 0.0
        for i in xrange(self.exp_profile.size()):
            iexp=self.exp_profile.get_intensity(i)
            icalc=model_profile.get_intensity(i)
            s2W += wx[i]*log(iexp/(icalc*gammahat))**2
        expected = log(s2W)*((self.exp_profile.size()-1)/2.0) - \
                log(gammainc((self.exp_profile.size()-1)/2., s2W/2.))
        return expected

    def test_contribs(self):
        "compute number of contributions of current model"
        self.perturb_particles()
        expected = self.exp_profile.size()
        self.sr.evaluate(None)
        observed=self.sr.get_number_of_contributions()
        self.assertEqual(observed,expected)

    def test_W(self):
        "compute W of current model"
        self.perturb_particles()
        wx = [(self.exp_profile.get_intensity(i)/self.exp_profile.get_error(i))**2
                for i in xrange(self.exp_profile.size())]
        gammahat=0.0
        W=sum(wx)
        expected = W
        self.sr.evaluate(None)
        observed=self.sr.get_W()
        self.assertAlmostEqual(observed/expected,1,delta=0.001)

    def test_gammahat(self):
        "compute gammahat of current model"
        for i in xrange(10):
            self.perturb_particles()
            model_profile = IMP.saxs.Profile(self.qmin, self.qmax, self.dq)
            model_profile.calculate_profile(self.particles)
            wx = [(self.exp_profile.get_intensity(i)
                        /self.exp_profile.get_error(i))**2
                    for i in xrange(self.exp_profile.size())]
            gammahat=0.0
            W=sum(wx)
            for i in xrange(self.exp_profile.size()):
                    gammahat += log((self.exp_profile.get_intensity(i)/
                            model_profile.get_intensity(i)))*(wx[i]/W)
            expected = gammahat
            self.sr.evaluate(None)
            observed=self.sr.get_log_gammahat()
            self.assertAlmostEqual(observed/expected,1,delta=0.001)

    def test_SS(self):
        "compute SS of current model"
        for i in xrange(10):
            self.perturb_particles()
            model_profile = IMP.saxs.Profile(self.qmin, self.qmax, self.dq)
            model_profile.calculate_profile(self.particles)
            wx = [(self.exp_profile.get_intensity(i)
                /self.exp_profile.get_error(i))**2
                    for i in xrange(self.exp_profile.size())]
            gammahat=0.0
            W=sum(wx)
            for i in xrange(self.exp_profile.size()):
                    gammahat += log((self.exp_profile.get_intensity(i)/
                            model_profile.get_intensity(i)))*(wx[i]/W)
            expected = 0.0
            for i in xrange(self.exp_profile.size()):
                iexp=self.exp_profile.get_intensity(i)
                icalc=model_profile.get_intensity(i)
                expected += wx[i]*(log(iexp/icalc)-gammahat)**2
            self.sr.evaluate(None)
            observed=self.sr.get_SS()
            self.assertAlmostEqual(observed/expected,1,delta=0.001)

    def testSanityPE(self):
        "test if prob is exp(-score)"
        for i in xrange(10):
            self.perturb_particles()
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
        expected = self.particles
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
            observed =self.sr.get_probability()
            expected=self.compute_prob()
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1)
            else:
                self.assertEqual(observed,0)

    def testValueEProt(self):
        """tests the energy by shuffling the atom's coordinates"""
        for i in xrange(10):
            self.perturb_particles()
            observed =self.sr.evaluate(None)
            expected=self.compute_ene()
            if expected != 0:
                self.assertAlmostEqual(observed/expected,1,delta=0.001)
            else:
                self.assertEqual(observed,0)

if __name__ == '__main__':
    IMP.test.main()
