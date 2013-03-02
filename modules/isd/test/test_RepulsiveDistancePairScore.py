#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import RepulsiveDistancePairScore

#unit testing framework
import IMP.test

class Tests(IMP.test.TestCase):
    "simple test cases to check if RepulsiveDistancePairScore works"
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #IMP.base.set_log_level(IMP.MEMORY)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.p0=IMP.core.XYZR.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0),1))
        self.p1=IMP.core.XYZR.setup_particle(IMP.Particle(self.m),
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5,1,1),2))
        self.DA = IMP.DerivativeAccumulator()
        self.ps = RepulsiveDistancePairScore(0.0,1.0)
        self.pc = IMP.container.ListPairContainer(self.m)
        self.pc.add_particle_pair((self.p0,self.p1))
        self.m.add_restraint(IMP.container.PairsRestraint(self.ps,self.pc))

    def testValue(self):
        """tests if energy is a repulsive quadric with zero at van der waals
        contatact, and with spring constant 1/2.
        """
        p0=self.p0
        p1=self.p1
        for i in xrange(100):
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(-5,5) \
                    for i in range(3)]))
            p1.set_coordinates(IMP.algebra.Vector3D(*[uniform(-5,5) \
                    for i in range(3)]))
                #when decorated as XYZR, get_distance substracts the radii!!
            dist=IMP.core.get_distance(p0,p1)
            if dist > 0:
                expected = 0
            else:
                expected = 0.5*dist**4
            self.assertAlmostEqual(self.m.evaluate(False),expected,delta=0.001)

    def testDeriv(self):
        """tests if derivative is -2*(d-x0)**3"""
        p0=self.p0
        p1=self.p1
        for i in xrange(100):
            p0.set_coordinates(IMP.algebra.Vector3D(*[uniform(-3,3) \
                    for i in range(3)]))
            p1.set_coordinates(IMP.algebra.Vector3D(*[uniform(-3,3) \
                    for i in range(3)]))
            #p1.set_coordinates(IMP.algebra.Vector3D(0,0,-float(i)/10))
            dist=IMP.core.get_distance(p0,p1)
            if dist > 0:
                expected = 0
            else:
                expected = 2*dist**3
            delta = p0.get_coordinates()-p1.get_coordinates()
            delta = delta / delta.get_magnitude()
            self.m.evaluate(self.DA)
            for i in xrange(3):
                self.assertAlmostEqual(p0.get_derivatives()[i],
                        expected*delta[i], delta=0.001)
                self.assertAlmostEqual(p1.get_derivatives()[i],
                        -expected*delta[i], delta=0.001)



if __name__ == '__main__':
    IMP.test.main()
