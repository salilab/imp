from __future__ import print_function, division
import IMP
import IMP.test
import IMP.core
import IMP.pmi
import IMP.pmi.restraints
import IMP.pmi.restraints.proteomics

class Tests(IMP.test.TestCase):
    def test_fb_1(self):

        for i in range(0,11):
            for j in range(0,11):
                for k in range(0,11):
                    a=i/10
                    b=j/10
                    c=k/10
                    fb1=IMP.pmi.restraints.proteomics.FuzzyBoolean(a)
                    fb2=IMP.pmi.restraints.proteomics.FuzzyBoolean(b)
                    fb3=IMP.pmi.restraints.proteomics.FuzzyBoolean(c)
                    fb4=(fb1|fb2)&fb3
                    f=(1-(1-a)*(1-b))*c
                    self.assertEqual(f,fb4.evaluate())

        for i in range(0,11):
            for j in range(0,11):
                for k in range(0,11):
                    a=i/10
                    b=j/10
                    c=k/10
                    fb1=IMP.pmi.restraints.proteomics.FuzzyBoolean(a)
                    fb2=IMP.pmi.restraints.proteomics.FuzzyBoolean(b)
                    fb3=IMP.pmi.restraints.proteomics.FuzzyBoolean(c)
                    fb4=(fb1&fb2)&fb3
                    f=(a*b)*c
                    self.assertEqual(f,fb4.evaluate())

        for i in range(0,11):
            for j in range(0,11):
                for k in range(0,11):
                    a=i/10
                    b=j/10
                    c=k/10
                    fb1=IMP.pmi.restraints.proteomics.FuzzyBoolean(a)
                    fb2=IMP.pmi.restraints.proteomics.FuzzyBoolean(b)
                    fb3=IMP.pmi.restraints.proteomics.FuzzyBoolean(c)
                    fb4=fb1|fb2|fb3
                    f=1-(1-(1-(1-a)*(1-b)))*(1-c)
                    s=1-(1-a)*(1-b)*(1-c)
                    self.assertEqual(f,fb4.evaluate())
                    self.assertAlmostEqual(s,fb4.evaluate())

    def test_fr_1(self):
        m=IMP.Model()
        p1=IMP.Particle(m)
        d1=IMP.core.XYZ.setup_particle(p1)
        d1.set_coordinates((0,0,0))
        p2=IMP.Particle(m)
        d2=IMP.core.XYZ.setup_particle(p2)
        d2.set_coordinates((5,0,0))
        p3=IMP.Particle(m)
        d3=IMP.core.XYZ.setup_particle(p3)
        d3.set_coordinates((10,0,0))

        pf=[]
        for i in range(10):
            p1=IMP.Particle(m)
            d1=IMP.core.XYZ.setup_particle(p1)
            d1.set_coordinates((i*5,0,0))
            pf.append(p1)

        pm=[]
        for i in range(3):
            p1=IMP.Particle(m)
            d1=IMP.core.XYZ.setup_particle(p1)
            d1.set_coordinates((i*5,0,0))
            pm.append(p1)

        r1a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[0],pm[0])
        r1b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[1],pm[1])
        r2a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[1],pm[0])
        r2b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[2],pm[1])
        r3a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[2],pm[0])
        r3b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[3],pm[1])
        r4a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[5],pm[0])
        r4b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,pf[6],pm[1])
        import numpy

        r=(r1a&r1b)|(r2a&r2b)|(r3a&r3b)|(r4a&r4b)
        for x in numpy.linspace(-1000, 1000, 1000):
            IMP.core.XYZ(pm[0]).set_coordinates((x,0,0))
            IMP.core.XYZ(pm[1]).set_coordinates((x+5,0,0))
            print(x,r.evaluate())

    def test_fr_2(self):
        m=IMP.Model()
        p1=IMP.Particle(m)
        d1=IMP.core.XYZ.setup_particle(p1)
        d1.set_coordinates((0,0,0))
        p2=IMP.Particle(m)
        d2=IMP.core.XYZ.setup_particle(p2)
        d2.set_coordinates((2,0,0))
        p3=IMP.Particle(m)
        d3=IMP.core.XYZ.setup_particle(p3)
        d3.set_coordinates((4,0,0))

        r1a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,p1,p2)
        r1b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,p1,p3)

        dr=r1a&(~r1b)
        nr1b=~r1b
        import numpy
        for x in numpy.linspace(-100, 100, 1000):
            IMP.core.XYZ(p1).set_coordinates((x,0,0))



    def test_xyz_particles_1(self):
        import IMP.rmf
        import RMF
        m=IMP.Model()
        ps=[]
        hs=[]
        for i in range(12):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates((i*5.0,0,0))
            d.set_radius(1)
            ps.append(p)
            hs.append(h)
            IMP.atom.Mass.setup_particle(p,1.0)
        IMP.core.XYZ(ps[11]).set_coordinates((-5.0,0,0))


        r1a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[0],ps[10])
        r1b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[9],ps[11])
        r2a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[1],ps[10])
        r2b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[8],ps[11])
        r3a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[2],ps[10])
        r3b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[7],ps[11])
        r4a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[3],ps[10])
        r4b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[6],ps[11])
        r5a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[4],ps[10])
        r5b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[5],ps[11])

        dr1=(r1a&r1b|r2a&r2b|r3a&r3b|r4a&r4b|r5a&r5b)

        r1a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[0],ps[10])
        r1b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[1],ps[11])
        r2a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[1],ps[10])
        r2b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[2],ps[11])
        r3a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[2],ps[10])
        r3b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[3],ps[11])
        r4a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[3],ps[10])
        r4b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[4],ps[11])
        r5a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[4],ps[10])
        r5b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[5],ps[11])

        dr2=(r1a&r1b|r2a&r2b|r3a&r3b|r4a&r4b|r5a&r5b)

        dr=dr1|dr2

        mvs=[]
        IMP.core.XYZ(ps[10]).set_coordinates_are_optimized(True)
        mvs.append(IMP.core.BallMover([ps[10]],1))
        IMP.core.XYZ(ps[11]).set_coordinates_are_optimized(True)
        mvs.append(IMP.core.BallMover([ps[11]],1))

        smv = IMP.core.SerialMover(mvs)
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function([dr])
        mc.set_return_best(False)
        mc.set_scoring_function
        mc.set_kt(1.0)
        mc.add_mover(smv)

        rh = RMF.create_rmf_file("test_fuzzy_restraint_1.rmf3")
        IMP.rmf.add_hierarchies(rh, hs)

        for n in range(100):
            mc.optimize(10*len(mvs))
            IMP.rmf.save_frame(rh)

        del rh

    def test_xyz_particles_2(self):
        import IMP.rmf
        import RMF
        m=IMP.Model()
        ps=[]
        hs=[]
        for i in range(13):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates((i*5.0,0,0))
            d.set_radius(1)
            ps.append(p)
            hs.append(h)
            IMP.atom.Mass.setup_particle(p,1.0)
        r1a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[0],ps[10])
        r1b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[1],ps[11])
        r1c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[2],ps[12])
        r2a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[1],ps[10])
        r2b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[2],ps[11])
        r2c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[3],ps[12])
        r3a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[2],ps[10])
        r3b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[3],ps[11])
        r3c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[4],ps[12])
        r4a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[3],ps[10])
        r4b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[4],ps[11])
        r4c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[5],ps[12])
        r5a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[4],ps[10])
        r5b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[5],ps[11])
        r5c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[6],ps[12])
        r6a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[5],ps[10])
        r6b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[6],ps[11])
        r6c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[7],ps[12])
        r7a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[6],ps[10])
        r7b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[7],ps[11])
        r7c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[8],ps[12])
        r8a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[7],ps[10])
        r8b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[8],ps[11])
        r8c=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[9],ps[12])


        dr=(r1a&r1b&r1c|r2a&r2b&r2c|r3a&r3b&r3c|r4a&r4b&r4c|r5a&r5b&r5c|r6a&r6b&r6c|r7a&r7b&r7c|r8a&r8b&r8c)

        mvs=[]
        IMP.core.XYZ(ps[10]).set_coordinates_are_optimized(True)
        mvs.append(IMP.core.BallMover([ps[10]],1))
        IMP.core.XYZ(ps[11]).set_coordinates_are_optimized(True)
        mvs.append(IMP.core.BallMover([ps[11]],1))
        IMP.core.XYZ(ps[12]).set_coordinates_are_optimized(True)
        mvs.append(IMP.core.BallMover([ps[12]],1))

        smv = IMP.core.SerialMover(mvs)
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function([dr])
        mc.set_return_best(False)
        mc.set_scoring_function
        mc.set_kt(1.0)
        mc.add_mover(smv)

        rh = RMF.create_rmf_file("test_fuzzy_restraint_2.rmf3")
        IMP.rmf.add_hierarchies(rh, hs)

        for n in range(100):
            mc.optimize(10*len(mvs))
            IMP.rmf.save_frame(rh)

        del rh

    def test_xyz_particles_3(self):
        '''
        testing the NOT
        '''
        import IMP.rmf
        import RMF
        m=IMP.Model()
        ps=[]
        hs=[]
        for i in range(3):
            p=IMP.Particle(m)
            h=IMP.atom.Hierarchy.setup_particle(p)
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates((i*5.0,0,0))
            d.set_radius(1)
            ps.append(p)
            hs.append(h)
            IMP.atom.Mass.setup_particle(p,1.0)
        r1a=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[0],ps[2])
        r1b=IMP.pmi.restraints.proteomics.FuzzyRestraint(m,ps[1],ps[2])

        dr=r1a&(~r1b)

        mvs=[]
        IMP.core.XYZ(ps[2]).set_coordinates_are_optimized(True)
        mvs.append(IMP.core.BallMover([ps[2]],1))

        smv = IMP.core.SerialMover(mvs)
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function([dr])
        mc.set_return_best(False)
        mc.set_scoring_function
        mc.set_kt(1.0)
        mc.add_mover(smv)

        rh = RMF.create_rmf_file("test_fuzzy_restraint_3.rmf3")
        IMP.rmf.add_hierarchies(rh, hs)

        for n in range(100):
            mc.optimize(10*len(mvs))
            IMP.rmf.save_frame(rh)

        del rh

if __name__ == '__main__':
    IMP.test.main()
