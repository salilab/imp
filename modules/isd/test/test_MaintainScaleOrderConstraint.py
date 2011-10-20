#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,JeffreysRestraint,MaintainScaleOrderConstraint

#unit testing framework
import IMP.test

class TestMaintainScaleOrderConstraint(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.set_log_level(IMP.MEMORY)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.n_sigmas = 20
        self.lower=0.1
        self.upper=100
        self.sigmas = [Scale.setup_particle(IMP.Particle(self.m), 1.0+i,
                        self.lower,self.upper) for i in xrange(self.n_sigmas)]
        self.J = [IMP.isd.JeffreysRestraint(s) for s in self.sigmas]
        for j in self.J:
            self.m.add_restraint(j)
        self.so=MaintainScaleOrderConstraint(self.sigmas)
        self.m.add_score_state(self.so)

    def testParticles(self):
        self.assertEqual(self.so.get_input_particles(),self.sigmas)

    def testContainers(self):
        self.assertEqual(self.so.get_input_containers(),[])

    def testOrdering(self):
        "tests if randomly shuffled scales are nicely sorted after that"
        for rep in xrange(100):
            scales = [uniform(self.lower, self.upper) for i in xrange(self.n_sigmas)]
            for (i,j) in  zip(self.sigmas,scales):
                i.set_scale(j)
            expected = sorted(scales)
            self.m.evaluate(None)
            observed = [ i.get_scale() for i in self.sigmas ]
            for (i,j) in zip(expected,observed):
                if j != 0:
                    self.assertAlmostEqual(i/j,1,delta=1e-4)
                else:
                    self.assertEqual(i,0)

    def contain(self, num):
        if num < self.lower:
            return self.lower
        if num > self.upper:
            return self.upper
        return num

    def testOrdering2(self):
        """tests if randomly shuffled scales are nicely sorted after that. Also
        tests bounds.
        """
        for rep in xrange(100):
            scales = [uniform(-10,2*self.upper) for i in xrange(self.n_sigmas)]
            for (i,j) in  zip(self.sigmas,scales):
                i.get_particle().set_value(IMP.FloatKey("nuisance"), j)
            expected = sorted([self.contain(i) for i in scales])
            self.m.evaluate(None)
            observed = [ i.get_scale() for i in self.sigmas ]
            for (i,j) in zip(expected,observed):
                if j != 0:
                    self.assertAlmostEqual(i/j,1,delta=1e-4)
                else:
                    self.assertEqual(i,0)

if __name__ == '__main__':
    IMP.test.main()
