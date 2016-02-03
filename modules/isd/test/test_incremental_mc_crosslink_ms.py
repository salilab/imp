from __future__ import print_function
import IMP.core
import IMP.algebra
import IMP.isd
import IMP.test

class Tests(IMP.test.TestCase):
    def test_incremental_sanity(self):
        """Test ISD restraints functionality and sanity with incremental MC scoring"""
        num_balls = 2
        num_mc_steps = 10
        m = IMP.Model()
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(30, 30, 30))

        p1=IMP.Particle(m)
        p1i=p1.get_index()
        d = IMP.core.XYZR.setup_particle(p1)
        d.set_radius(10)
        d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
        d.set_coordinates_are_optimized(True)

        p2=IMP.Particle(m)
        p2i=p2.get_index()
        d = IMP.core.XYZR.setup_particle(p2)
        d.set_radius(10)
        d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
        d.set_coordinates_are_optimized(True)

        psigma=IMP.Particle(m)
        s1i=psigma.get_index()
        s2i=psigma.get_index()
        sigma = IMP.isd.Nuisance.setup_particle(psigma)
        sigma.set_lower(0.)
        sigma.set_nuisance(10)
        sigma.set_nuisance_is_optimized(True)

        ppsi=IMP.Particle(m)
        psii=ppsi.get_index()
        psi = IMP.isd.Nuisance.setup_particle(ppsi)
        psi.set_lower(0.)
        psi.set_nuisance(0.05)
        psi.set_nuisance_is_optimized(True)

        dr = IMP.isd.CrossLinkMSRestraint(
                                    m,
                                    21.0,
                                    0.01)

        dr.add_contribution((p1i, p2i), (s1i, s2i), psii)

        mc = IMP.core.MonteCarlo(m)
        isf = IMP.core.IncrementalScoringFunction([p1,p2,psigma,ppsi], [dr])
        mc.set_incremental_scoring_function(isf)

        mvs = [IMP.core.BallMover([p], 5) for p in [p1,p2]]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        initial=isf.evaluate(False)
        after = mc.optimize(num_mc_steps)

    def test_incremental_score(self):
        """Test ISD restraints with incremental MC scoring score is indentical"""

        import time
        num_balls = 2
        num_mc_steps = 10
        m = IMP.Model()
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(30, 30, 30))

        psigma=IMP.Particle(m)
        s1i=psigma.get_index()
        s2i=psigma.get_index()
        sigma = IMP.isd.Nuisance.setup_particle(psigma)
        sigma.set_lower(0.)
        sigma.set_nuisance(10)
        sigma.set_nuisance_is_optimized(True)

        ppsi=IMP.Particle(m)
        psii=ppsi.get_index()
        psi = IMP.isd.Nuisance.setup_particle(ppsi)
        psi.set_lower(0.)
        psi.set_nuisance(0.05)
        psi.set_nuisance_is_optimized(True)


        ps=[]
        restraints=[]
        for n in range(10):

            dr = IMP.isd.CrossLinkMSRestraint(
                                    m,
                                    21.0,
                                    0.01)

            p1=IMP.Particle(m)
            p1i=p1.get_index()
            d = IMP.core.XYZR.setup_particle(p1)
            d.set_radius(10)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)

            p2=IMP.Particle(m)
            p2i=p2.get_index()
            d = IMP.core.XYZR.setup_particle(p2)
            d.set_radius(10)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)

            dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
            ps+=[p1,p2]
            restraints.append(dr)


        mc = IMP.core.MonteCarlo(m)
        sf = IMP.core.RestraintsScoringFunction(restraints)
        isf = IMP.core.IncrementalScoringFunction(ps+[psigma,ppsi], restraints)
        mc.set_incremental_scoring_function(isf)

        mvs = [IMP.core.BallMover([p], 5) for p in ps]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        self.assertAlmostEqual(isf.evaluate(False),sf.evaluate(False))
        starttime1 = time.clock()
        mc.optimize(num_mc_steps*len(ps))
        endtime1 = time.clock()
        self.assertAlmostEqual(isf.evaluate(False),sf.evaluate(False))

        # create a new montecarlo simulation with the standard scoring function
        mc = IMP.core.MonteCarlo(m)
        sf = IMP.core.RestraintsScoringFunction(restraints)
        mc.set_scoring_function(sf)

        mvs = [IMP.core.BallMover([p], 5) for p in ps]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        starttime2 = time.clock()
        mc.optimize(num_mc_steps*len(ps))
        endtime2 = time.clock()

        # check that the time spent using the normal scoring function is larger
        # than the time spent using the incremental one
        self.assertLess(endtime1-starttime1, endtime2-starttime2)


    def test_incremental_score_optimization(self):
        """Test ISD restraints with incremental MC score decreases"""

        import time
        num_mc_steps = 10
        m = IMP.Model()
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(30, 30, 30))

        psigma=IMP.Particle(m)
        s1i=psigma.get_index()
        s2i=psigma.get_index()
        sigma = IMP.isd.Nuisance.setup_particle(psigma)
        sigma.set_lower(0.)
        sigma.set_nuisance(10)
        sigma.set_nuisance_is_optimized(True)

        ppsi=IMP.Particle(m)
        psii=ppsi.get_index()
        psi = IMP.isd.Nuisance.setup_particle(ppsi)
        psi.set_lower(0.)
        psi.set_nuisance(0.05)
        psi.set_nuisance_is_optimized(True)


        ps=[]
        restraints=[]
        for n in range(10):

            dr = IMP.isd.CrossLinkMSRestraint(
                                    m,
                                    21.0,
                                    0.01)

            p1=IMP.Particle(m)
            p1i=p1.get_index()
            d = IMP.core.XYZR.setup_particle(p1)
            d.set_radius(10)
            IMP.atom.Mass.setup_particle(p1,100)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)

            p2=IMP.Particle(m)
            p2i=p2.get_index()
            d = IMP.core.XYZR.setup_particle(p2)
            d.set_radius(10)
            IMP.atom.Mass.setup_particle(p2,100)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)

            dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
            ps+=[p1,p2]
            restraints.append(dr)


        mc = IMP.core.MonteCarlo(m)
        isf = IMP.core.IncrementalScoringFunction(ps+[psigma,ppsi], restraints)
        mc.set_incremental_scoring_function(isf)
        sf = IMP.core.RestraintsScoringFunction(restraints)

        mvs = [IMP.core.BallMover([p], 5) for p in ps]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        for i in range(10):
            mc.optimize(len(ps))
            print(isf.evaluate(False))


    def test_incremental_score_rigid_bodies(self):
        """Test ISD restraints with incremental MC score decreases"""

        import time
        import random

        num_mc_steps = 10
        m = IMP.Model()
        bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                       IMP.algebra.Vector3D(30, 30, 30))

        psigma=IMP.Particle(m)
        s1i=psigma.get_index()
        s2i=psigma.get_index()
        sigma = IMP.isd.Nuisance.setup_particle(psigma)
        sigma.set_lower(0.)
        sigma.set_nuisance(10)
        sigma.set_nuisance_is_optimized(True)

        ppsi=IMP.Particle(m)
        psii=ppsi.get_index()
        psi = IMP.isd.Nuisance.setup_particle(ppsi)
        psi.set_lower(0.)
        psi.set_nuisance(0.05)
        psi.set_nuisance_is_optimized(True)


        ps=[]
        restraints=[]
        for n in range(10):

            dr = IMP.isd.CrossLinkMSRestraint(
                                    m,
                                    21.0,
                                    0.01)

            p1=IMP.Particle(m)
            p1i=p1.get_index()
            d = IMP.core.XYZR.setup_particle(p1)
            d.set_radius(10)
            IMP.atom.Mass.setup_particle(p1,100)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)

            p2=IMP.Particle(m)
            p2i=p2.get_index()
            d = IMP.core.XYZR.setup_particle(p2)
            d.set_radius(10)
            IMP.atom.Mass.setup_particle(p2,100)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_coordinates_are_optimized(True)

            dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
            ps+=[p1,p2]
            restraints.append(dr)

        ps1=ps[:int(len(ps)/2)]
        ps2=ps[int(len(ps)/2):]
        rb1 = IMP.atom.create_rigid_body(ps1)
        rb1.set_coordinates_are_optimized(True)
        rb2 = IMP.atom.create_rigid_body(ps2)
        rb2.set_coordinates_are_optimized(True)

        mc = IMP.core.MonteCarlo(m)
        print(list(set(restraints)))
        isf = IMP.core.IncrementalScoringFunction([rb1.get_particle(),rb2.get_particle()]
                                                +ps+[psigma,ppsi], list(set(restraints)))
        mc.set_incremental_scoring_function(isf)

        mvs = [IMP.core.RigidBodyMover(rb1, 1.0, 0.03),IMP.core.RigidBodyMover(rb2, 1.0, 0.03)]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        for i in range(10):
            mc.optimize(len(mvs))
            print(isf.evaluate(False))




if __name__ == '__main__':
    IMP.test.main()
