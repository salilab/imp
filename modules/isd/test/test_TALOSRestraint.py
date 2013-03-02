#!/usr/bin/env python

#general imports
from math import *
import math
from random import gauss,uniform,randint

#imp general
import IMP

#our project
from IMP.isd import Scale,TALOSRestraint

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    "simple test cases to check if TALOSRestraint works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        #setup dihedral angle of pi/2
        self.p0 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                IMP.algebra.Vector3D(1,0,0))
        self.p1 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                IMP.algebra.Vector3D(0,0,0))
        self.p2 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                IMP.algebra.Vector3D(0,1,0))
        self.p3 = IMP.core.XYZ.setup_particle(IMP.Particle(self.m),
                IMP.algebra.Vector3D(0,1,1))
        #scale particle
        self.kappa = Scale.setup_particle(IMP.Particle(self.m), 2.0)
        self.DA = IMP.DerivativeAccumulator()

    def setup_restraint(self):
        N=randint(1,20)
        self.N=N
        self.meanv=uniform(-pi,pi)
        self.stdev=uniform(0,2*pi)
        self.obs = [gauss(self.meanv,self.stdev) for i in xrange(N)]
        self.cosbar = sum([math.cos(x) for x in self.obs])
        self.sinbar = sum([math.sin(x) for x in self.obs])
        self.R=sqrt(self.cosbar**2+self.sinbar**2)
        R=self.R
        self.chiexp=acos(self.cosbar/R)
        if self.sinbar <0:
            self.chiexp = -self.chiexp
        self.talos = IMP.isd.TALOSRestraint(self.p0, self.p1, self.p2, self.p3,
                self.obs, self.kappa)
        #self.talos = IMP.isd.TALOSRestraint(self.p0, self.p1, self.p2, self.p3,
        #        N, self.R, self.chiexp, self.kappa)
        self.m.add_restraint(self.talos)

    def testAlternatives(self):
        "Test make TALOSRestraint with particle list and sufficient stats"
        self.setup_restraint()
        talos1=IMP.isd.TALOSRestraint([self.p0, self.p1, self.p2, self.p3],
                self.N, self.R, self.chiexp, self.kappa)
        self.m.add_restraint(talos1)
        self.assertAlmostEqual(self.talos.evaluate(None), talos1.evaluate(None),
                               delta=1e-5)

    def testAlternatives2(self):
        "Test make TALOSRestraint with 4 particles and the data"
        self.setup_restraint()
        talos1=IMP.isd.TALOSRestraint(self.p0, self.p1, self.p2, self.p3,
                self.obs, self.kappa)
        self.m.add_restraint(talos1)
        self.assertAlmostEqual(self.talos.evaluate(None), talos1.evaluate(None),
                               delta=1e-5)

    def testAlternatives3(self):
        "Test make TALOSRestraint with particle list and the data"
        self.setup_restraint()
        talos1=IMP.isd.TALOSRestraint([self.p0, self.p1, self.p2, self.p3],
                self.obs, self.kappa)
        self.m.add_restraint(talos1)
        self.assertAlmostEqual(self.talos.evaluate(None), talos1.evaluate(None),
                               delta=1e-5)

    def testValueDDist(self):
        "Test TALOS derivatives for the angle using a small CG minimization"
        self.N=10
        self.R=3
        self.chiexp = pi/3
        self.talos = IMP.isd.TALOSRestraint(self.p0,self.p1, self.p2, self.p3,
                self.N, self.R, self.chiexp, self.kappa)
        self.m.add_restraint(self.talos)
        #constrain particles to a fixed "bondlength" of 1
        uf=IMP.core.Harmonic(1,100)
        df = IMP.core.DistancePairScore(uf)
        self.m.add_restraint(
                IMP.core.PairRestraint(df, (self.p0,self.p1)))
        self.m.add_restraint(
                IMP.core.PairRestraint(df, (self.p1,self.p2)))
        self.m.add_restraint(
                IMP.core.PairRestraint(df, (self.p2,self.p3)))
        self.p3.set_coordinates_are_optimized(True)
        self.p2.set_coordinates_are_optimized(False)
        self.p1.set_coordinates_are_optimized(False)
        self.p0.set_coordinates_are_optimized(False)
        cg=IMP.core.ConjugateGradients(self.m)
        for i in xrange(10):
            x=i/(2*pi)
            self.p3.set_coordinates(IMP.algebra.Vector3D(
                cos(2*pi-x),1,sin(2*pi-x)))
            kappa = uniform(1,5)
            self.kappa.set_scale(kappa)
            cg.optimize(100)
            self.talos.evaluate(self.DA)
            pos=self.p3.get_coordinates()
            obssin=-pos[2]/sqrt(pos[0]**2+pos[2]**2)
            obscos=pos[0]/sqrt(pos[0]**2+pos[2]**2)
            self.assertAlmostEqual(obscos,cos(pi/3),delta=1e-6)
            self.assertAlmostEqual(obssin,sin(pi/3),delta=1e-6)

    def testValueDKappa1(self):
        """Test TALOS derivatives for kappa by varying kappa"""
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        self.setup_restraint()
        self.p3.set_coordinates(IMP.algebra.Vector3D(0,1,-1))
        for i in xrange(100):
            kappa = uniform(0.1,10)
            self.kappa.set_scale(kappa)
            self.talos.evaluate(self.DA)
            py=self.N*i1(kappa)/i0(kappa) - self.R*cos(pi/2-self.chiexp)
            cpp=self.kappa.get_scale_derivative()
            if py == 0:
                self.assertEqual(cpp,0)
            else:
                self.assertAlmostEqual(cpp/py,1.0,delta=1e-6)

    def testValueDKappa2(self):
        """Test TALOS derivatives for kappa by varying the angle"""
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        self.setup_restraint()
        for i in xrange(100):
            x=i/(2*pi)
            self.p3.set_coordinates(IMP.algebra.Vector3D(
                cos(2*pi-x),1,sin(2*pi-x)))
            kappa = self.kappa.get_scale()
            self.talos.evaluate(self.DA)
            py=self.N*i1(kappa)/i0(kappa) - self.R*cos(x-self.chiexp)
            cpp=self.kappa.get_scale_derivative()
            if py == 0:
                self.assertEqual(cpp,0)
            else:
                self.assertAlmostEqual(cpp/py,1.0,delta=1e-6)

    def testValueEDist(self):
        """Test energy of TALOSRestraint by varying p3"""
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        self.setup_restraint()
        for i in xrange(100):
            x=i/(2*pi)
            self.p3.set_coordinates(IMP.algebra.Vector3D(
                cos(2*pi-x),1,sin(2*pi-x)))
            kappa = self.kappa.get_scale()
            cpp=self.talos.evaluate(None)
            py=log(2*pi*i0(kappa)**self.N) - self.R*kappa*cos(x-self.chiexp)
            if py == 0:
                self.assertEqual(cpp,0)
            else:
                self.assertAlmostEqual(cpp/py,1.0,delta=1e-6)

    def testValueEKappa(self):
        """Test energy of TALOSRestraint by varying kappa"""
        try:
            from scipy.special import i0,i1
        except ImportError:
            self.skipTest("this test requires the scipy Python module")
        self.setup_restraint()
        self.p3.set_coordinates(IMP.algebra.Vector3D(0,1,-1))
        for i in xrange(100):
            kappa = uniform(0.1,10)
            self.kappa.set_scale(kappa)
            cpp=self.talos.evaluate(None)
            py=log(2*pi*i0(kappa)**self.N) - self.R*kappa*cos(pi/2-self.chiexp)
            if py == 0:
                self.assertEqual(cpp,0)
            else:
                self.assertAlmostEqual(cpp/py,1.0,delta=1e-6)

    def testParticles(self):
        "Test TALOSRestraint::get_input_particles"
        self.setup_restraint()
        self.assertEqual(self.talos.get_input_particles(),
                [self.p0,self.p1,self.p2,self.p3,self.kappa])

    def testContainers(self):
        "Test TALOSRestraint::get_input_containers"
        self.setup_restraint()
        self.assertEqual(self.talos.get_input_containers(),[])

    def testSanityEP(self):
        "Test if TALOS score is -log(prob)"
        self.setup_restraint()
        for i in xrange(100):
            no=uniform(0.1,10)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.talos.unprotected_evaluate(self.DA),
                    -log(self.talos.get_probability()),delta=0.001)

    def testSanityPE(self):
        "Test if TALOS prob is exp(-score)"
        self.setup_restraint()
        for i in xrange(100):
            no=uniform(0.1,10)
            self.kappa.set_scale(no)
            self.assertAlmostEqual(self.talos.get_probability(),
                    exp(-self.talos.unprotected_evaluate(self.DA)),delta=0.001)

    def testModeKappa(self):
        """Test TALOSRestraint via CG on kappa.
            For N=10 cos(chi)=1 and R=9, the mode should be
            E = -10.125165226189658481
            kappa = 5.3046890629577175140
        """
        self.N=10
        self.R=9
        self.chiexp = -pi/2
        self.talos = IMP.isd.TALOSRestraint(self.p0,self.p1, self.p2, self.p3,
                self.N, self.R, self.chiexp, self.kappa)
        self.m.add_restraint(self.talos)
        #constrain particles to a fixed "bondlength" of 1
        self.p3.set_coordinates_are_optimized(False)
        self.p2.set_coordinates_are_optimized(False)
        self.p1.set_coordinates_are_optimized(False)
        self.p0.set_coordinates_are_optimized(False)
        self.kappa.set_is_optimized(self.kappa.get_scale_key(), True)
        cg=IMP.core.ConjugateGradients(self.m)
        self.kappa.set_scale(3)
        cg.optimize(100)
        kappa=self.kappa.get_scale()
        E=self.m.evaluate(None)
        expE = -10.125165226189658481
        expkappa = 5.3046890629577175140
        self.assertAlmostEqual(kappa,expkappa,delta=1e-6)
        self.assertAlmostEqual(E,expE,delta=1e-6)

if __name__ == '__main__':
    IMP.test.main()
