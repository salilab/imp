#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP

#our project
from IMP.isd import Scale,HybridMonteCarlo

#unit testing framework
import IMP.test

vel_keys_xyz=[IMP.FloatKey("vx"), IMP.FloatKey("vy"), IMP.FloatKey("vz")]
vel_key_nuisance=IMP.FloatKey("vel")
kB=1.381*6.02214/4184.0

class TestHybridMonteCarlo(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.xyzs=[]
        self.nuisances=[]
        self.restraints=[]
        self.setup_system()
        self.setup_HMC(temp=1/kB)

    def setup_xyz(self,coords, mass):
        a=IMP.Particle(self.m)
        IMP.core.XYZ.setup_particle(a, coords)
        IMP.core.XYZ(a).set_coordinates_are_optimized(True)
        IMP.atom.Mass.setup_particle(a, mass)
        return a

    def setup_scale(self,coords, mass):
        a=IMP.Particle(self.m)
        IMP.isd.Scale.setup_particle(a, coords)
        IMP.isd.Scale(a).set_scale_is_optimized(True)
        IMP.atom.Mass.setup_particle(a, mass)
        return a

    def setup_system(self):
        """setup two xyzs and two nuisances linked by a Lognormal restraint"""
        a=self.setup_xyz(IMP.algebra.Vector3D((0,0,0)),1.0)
        b=self.setup_xyz(IMP.algebra.Vector3D((1,1,1)),1.0)
        si=self.setup_scale(1.0, 1.0)
        ga=self.setup_scale(1.0, 1.0)
        ln=IMP.isd.NOERestraint(a,b,si,ga,1.0)
        self.m.add_restraint(ln)
        self.xyzs.append(a)
        self.xyzs.append(b)
        self.nuisances.append(si)
        self.nuisances.append(ga)
        self.restraints.append(ln)

    def setup_HMC(self, temp, nsteps=10, tstep=1.0, persistence=1):
        self.hmc = IMP.isd.HybridMonteCarlo(self.m, kB*temp, nsteps, tstep,
                persistence)
        self.hmc.set_was_used(True)

    def get_nuisance_coordinates(self):
        a=[i.get_value(IMP.isd.Scale.get_scale_key())
                for i in self.nuisances]
        b=[i.get_value(vel_key_nuisance) for i in self.nuisances]
        return a+b

    def get_xyz_coordinates(self):
        a=[[i.get_value(fl) for fl in IMP.core.XYZ.get_xyz_keys()]
                for i in self.xyzs]
        b=[[i.get_value(fl) for fl in vel_keys_xyz]
                for i in self.xyzs]
        return a+b

    def test_move(self):
        """test that the HMC moves the particles"""
        self.hmc.get_md().optimize(0)
        oldn = self.get_nuisance_coordinates()
        oldx = self.get_xyz_coordinates()
        self.hmc.optimize(1)
        newn = self.get_nuisance_coordinates()
        newx = self.get_xyz_coordinates()
        for i,j in zip(newx,oldx):
            self.assertNotAlmostEqual(i[0],j[0],delta=1e-7)
            self.assertNotAlmostEqual(i[1],j[1],delta=1e-7)
            self.assertNotAlmostEqual(i[2],j[2],delta=1e-7)
        for i,j in zip(newn,oldn):
            self.assertNotAlmostEqual(i,j,delta=1e-7)

    def test_ensemble_analytical(self):
        """test ensemble averages"""
        IMP.isd.Nuisance(self.nuisances[0]).set_nuisance_is_optimized(False)
        IMP.isd.Nuisance(self.nuisances[1]).set_nuisance_is_optimized(False)
        IMP.core.XYZ(self.xyzs[0]).set_coordinates_are_optimized(False)
        IMP.core.XYZ(self.xyzs[1]).set_coordinates_are_optimized(True)
        self.hmc.get_md().optimize(0)
        pos=[]
        ene=[]
        for i in xrange(1000):
            self.hmc.optimize(1)
            pos.append(IMP.core.XYZ(self.xyzs[1]).get_coordinates())
            ene.append(self.m.evaluate(False))
        dist = [i.get_magnitude() for i in pos]
        #mean should be exp(s^2/24) g^(1/6)
        #e.g. exp(1/24)
        print mean(dist),exp(1/24.)
        self.assertAlmostEqual(mean(dist),exp(1/24.),delta=1e-2)
        #variance should be exp(s^2/12)(exp(s^2/36)-1)g^(1/3)
        #e.g. exp(1/12)(exp(1/36)-1)
        print var(dist), exp(1/12.)*(exp(1/36.)-1)
        self.assertAlmostEqual(var(dist), exp(1/12.)*(exp(1/36.)-1), delta=1e-2)
        #average energy should be 1/72(s^2+36*(1+log(2*pi))) + log(s)
        #e.g. 1/72(37+36*log(2*pi))
        print mean(ene), 1/72.*(37+36*log(2*pi))
        self.assertAlmostEqual(mean(ene), 1/72.*(37+36*log(2*pi)), delta=1e-2)

    def test_ensemble_mc(self):
        """test ensemble averages against the standard Monte Carlo"""
        IMP.isd.Nuisance(self.nuisances[0]).set_nuisance_is_optimized(False)
        IMP.isd.Nuisance(self.nuisances[1]).set_nuisance_is_optimized(False)
        IMP.core.XYZ(self.xyzs[0]).set_coordinates_are_optimized(False)
        IMP.core.XYZ(self.xyzs[1]).set_coordinates_are_optimized(True)
        self.hmc.get_md().optimize(0)
        #HMC
        pos=[]
        ene=[]
        for i in xrange(1000):
            self.hmc.optimize(1)
            pos.append(IMP.core.XYZ(self.xyzs[1]).get_coordinates())
            ene.append(self.m.evaluate(False))
        dist = [i.get_magnitude() for i in pos]
        #standard MC
        mc=IMP.core.MonteCarlo(self.m)
        mv=IMP.core.NormalMover([self.xyzs[1]], IMP.core.XYZ.get_xyz_keys(),
                0.1)
        mc.add_mover(mv)
        mc.set_return_best(False)
        mc.set_kt(1.0)
        pos2=[]
        ene2=[]
        for i in xrange(1000):
            mc.optimize(1)
            pos2.append(IMP.core.XYZ(self.xyzs[1]).get_coordinates())
            ene2.append(self.m.evaluate(False))
        dist2 = [i.get_magnitude() for i in pos2]
        self.assertAlmostEqual(mean(dist),mean(dist2),delta=1e-1)
        self.assertAlmostEqual(var(dist), var(dist2), delta=1e-1)
        self.assertAlmostEqual(mean(ene), mean(ene2), delta=1e-1)






if __name__ == '__main__':
    IMP.test.main()
