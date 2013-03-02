#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,JeffreysRestraint,MarginalHBondRestraint

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    "simple test cases to check if MarginalHBondRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.p0=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(0,0,0))
        self.p1=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(1,1,1))
        self.p2=IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Vector3D(1,0,0))
        self.DA = IMP.DerivativeAccumulator()
        self.V_obs=3.0
        self.ls = \
           IMP.container.ListPairContainer([(self.p0,self.p1),(self.p0,self.p2)])
        self.noe = IMP.isd.MarginalHBondRestraint()

    def testValuePDist1(self):
        """Test MarginalHBondRestraint probability on three particles"""
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        self.noe.add_contribution(self.p0,self.p1,1.0)
        self.noe.add_contribution(IMP.container.ListPairContainer([(self.p0,self.p2)]),2.0)
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            for p in [self.p0,self.p1,self.p2]:
                p.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            dist1=IMP.core.get_distance(p0,p1)**-6
            dist2=IMP.core.get_distance(p0,p2)**-6
            expected=1/(log(dist1/v1)**2 + log(dist2/v2)**2)
            self.assertAlmostEqual(self.noe.get_probability(),
                    expected,delta=0.001)

    def testValueEDist1(self):
        """Test MarginalHBondRestraint energy on three particles"""
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        self.noe.add_contribution(self.p0,self.p1,v1)
        self.noe.add_contribution(IMP.container.ListPairContainer([(self.p0,self.p2)]), v2)
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            for p in [self.p0,self.p1,self.p2]:
                p.set_coordinates(IMP.algebra.Vector3D(*[uniform(0.1,100) \
                    for i in range(3)]))
            dist1=IMP.core.get_distance(p0,p1)**-6
            dist2=IMP.core.get_distance(p0,p2)**-6
            expected=log(log(dist1/v1)**2 + log(dist2/v2)**2)
            self.noe.evaluate(None)
            self.assertAlmostEqual(self.noe.evaluate(None),
                    expected,delta=0.001)

    def testValuePDist2(self):
        """Test MarginalHBondRestraint probability on n particles"""
        pairs=[]
        volumes=[]
        distances=[]
        self.m.add_restraint(self.noe)
        for i in xrange(2,100):
            while len(pairs) <= i:
                pair=[IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                    IMP.algebra.Vector3D(*[uniform(-10,10) for r in range(3)]))
                        for p in range(2)]
                pairs.append(pair)
                distances.append(IMP.core.get_distance(pair[0],pair[1]))
                volumes.append(uniform(0.1,10))
                self.noe.add_contribution(IMP.container.ListPairContainer([pair]),
                        volumes[-1])
            expected = 0
            for j in xrange(len(pairs)):
                expected += log(volumes[j]/distances[j]**(-6))**2
            expected = expected ** (-len(volumes)/2.0)
            self.assertAlmostEqual(self.noe.get_probability(),
                    expected,delta=0.001)

    def testValueEDist2(self):
        """Test MarginalHBondRestraint energy on n particles"""
        pairs=[]
        volumes=[]
        distances=[]
        self.m.add_restraint(self.noe)
        for i in xrange(2,100):
            while len(pairs) <= i:
                pair=[IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                    IMP.algebra.Vector3D(*[uniform(-10,10) for r in range(3)]))
                        for p in range(2)]
                pairs.append(pair)
                distances.append(IMP.core.get_distance(pair[0],pair[1]))
                volumes.append(uniform(0.1,10))
                self.noe.add_contribution(IMP.container.ListPairContainer([pair]),
                        volumes[-1])
            expected = 0
            for j in xrange(len(pairs)):
                expected += log(volumes[j]/distances[j]**(-6))**2
            expected = log(expected) * (len(volumes)/2.0)
            self.assertAlmostEqual(self.noe.evaluate(None),
                    expected,delta=0.001)

    def testValueLogsquares(self):
        """Test MarginalHBondRestraint logsquares on n particles"""
        pairs=[]
        volumes=[]
        distances=[]
        self.m.add_restraint(self.noe)
        for i in xrange(2,100):
            while len(pairs) <= i:
                pair=[IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                    IMP.algebra.Vector3D(*[uniform(-10,10) for r in range(3)]))
                        for p in range(2)]
                pairs.append(pair)
                distances.append(IMP.core.get_distance(pair[0],pair[1]))
                volumes.append(uniform(0.1,10))
                self.noe.add_contribution(IMP.container.ListPairContainer([pair]),
                        volumes[-1])
            expected = 0
            for j in xrange(len(pairs)):
                expected += log(volumes[j]/distances[j]**(-6))**2
            self.noe.evaluate(None)
            self.assertAlmostEqual(self.noe.get_logsquares(),
                    expected,delta=0.001)

    def testValueN(self):
        """Test MarginalHBondRestraint n on n particles"""
        pairs=[]
        volumes=[]
        distances=[]
        self.m.add_restraint(self.noe)
        for i in xrange(2,100):
            while len(pairs) <= i:
                pair=[IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                    IMP.algebra.Vector3D(*[uniform(-10,10) for r in range(3)]))
                        for p in range(2)]
                pairs.append(pair)
                distances.append(IMP.core.get_distance(pair[0],pair[1]))
                volumes.append(uniform(0.1,10))
                self.noe.add_contribution(IMP.container.ListPairContainer([pair]),
                        volumes[-1])
            expected = len(volumes)
            self.assertAlmostEqual(self.noe.get_number_of_contributions(),
                    expected,delta=0.001)

    def testDerivative(self):
        "Test MarginalHBondRestraint x deriv for 3 particles & 2 contributions"
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        self.noe.add_contribution(self.p0,self.p1,v1)
        self.noe.add_contribution(IMP.container.ListPairContainer([(self.p0,self.p2)]),v2)
        self.m.add_restraint(self.noe)
        p0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        p1.set_coordinates(IMP.algebra.Vector3D(1,1,1))
        p2.set_coordinates(IMP.algebra.Vector3D(1,0,0))
        self.noe.evaluate(self.DA)
        #p0
        self.assertAlmostEqual(self.p0.get_derivative(0),
                   -1.89554,delta=0.001)
        self.assertAlmostEqual(self.p0.get_derivative(1),
                   -1.16225,delta=0.001)
        self.assertAlmostEqual(self.p0.get_derivative(2),
                   -1.16225,delta=0.001)
        #p1
        self.assertAlmostEqual(self.p1.get_derivative(0),
                    1.16225,delta=0.001)
        self.assertAlmostEqual(self.p1.get_derivative(1),
                    1.16225,delta=0.001)
        self.assertAlmostEqual(self.p1.get_derivative(2),
                    1.16225,delta=0.001)
        #p2
        self.assertAlmostEqual(self.p2.get_derivative(0),
                    0.73330,delta=0.001)
        self.assertAlmostEqual(self.p2.get_derivative(1),
                    0.,delta=0.001)
        self.assertAlmostEqual(self.p2.get_derivative(2),
                    0.,delta=0.001)

    def testParticles(self):
        "Test MarginalHBondRestraint::get_input_particles"
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        self.noe.add_contribution(self.p0,self.p1,v1)
        self.noe.add_contribution(IMP.container.ListPairContainer([(self.p0,self.p2)]),v2)
        self.assertEqual(self.noe.get_input_particles(),
                [self.p0,self.p1,self.p0,self.p2])

    def testContainers(self):
        "Test MarginalHBondRestraint::get_input_containers"
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        c1=IMP.container.ListPairContainer([(self.p0,self.p1)])
        c2=IMP.container.ListPairContainer([(self.p0,self.p2)])
        self.noe.add_contribution(c1,v1)
        self.noe.add_contribution(c2,v2)
        self.assertEqual(self.noe.get_input_containers(),[c1,c2])

    def testSanityEP(self):
        "Test if MarginalHBond score is -log(prob)"
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        c1=IMP.container.ListPairContainer([(self.p0,self.p1)])
        c2=IMP.container.ListPairContainer([(self.p0,self.p2)])
        self.noe.add_contribution(c1,v1)
        self.noe.add_contribution(c2,v2)
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(-10,10) for i in
                range(3)]))
            self.assertAlmostEqual(self.noe.evaluate(None),
                    -log(self.noe.get_probability()),delta=0.001)

    def testSanityPE(self):
        "Test if MarginalHBondRestraint prob is exp(-score)"
        v1,v2=1.0,2.0
        p0,p1,p2=self.p0,self.p1,self.p2
        c1=IMP.container.ListPairContainer([(self.p0,self.p1)])
        c2=IMP.container.ListPairContainer([(self.p0,self.p2)])
        self.noe.add_contribution(c1,v1)
        self.noe.add_contribution(c2,v2)
        self.m.add_restraint(self.noe)
        for i in xrange(100):
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(-10,10) for i in
                range(3)]))
            self.assertAlmostEqual(self.noe.get_probability(),
                    exp(-self.noe.evaluate(None)),delta=0.001)

if __name__ == '__main__':
    IMP.test.main()
