from __future__ import print_function
import IMP.core
import IMP.algebra
import IMP.isd
import IMP.test


def setup_system(coords):
    num_balls = 2
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
        dr = IMP.isd.CrossLinkMSRestraint(m, 21.0, 0.01)

        p1=IMP.Particle(m)
        p1i=p1.get_index()
        d = IMP.core.XYZR.setup_particle(p1)
        d.set_radius(10)
        d.set_coordinates(coords[n*2])
        d.set_coordinates_are_optimized(True)

        p2=IMP.Particle(m)
        p2i=p2.get_index()
        d = IMP.core.XYZR.setup_particle(p2)
        d.set_radius(10)
        d.set_coordinates(coords[n*2+1])
        d.set_coordinates_are_optimized(True)

        dr.add_contribution((p1i, p2i), (s1i, s2i), psii)
        ps+=[p1,p2]
        restraints.append(dr)

    mc = IMP.core.MonteCarlo(m)
    sf = IMP.core.RestraintsScoringFunction(restraints)
    mc.set_scoring_function(sf)

    mvs = [IMP.core.BallMover(m, [p], 5) for p in ps]
    sm = IMP.core.SerialMover(mvs)
    mc.add_mover(sm)
    # Always reject upwards moves
    mc.set_kt(0.)
    mc.set_return_best(False)
    return m, mc


class Tests(IMP.test.TestCase):
    def test_moved_sanity(self):
        """Test ISD restraints functionality and sanity with
           move-aware MC scoring"""
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
        sf = IMP.core.RestraintsScoringFunction([dr])
        mc.set_scoring_function(sf)
        mc.set_score_moved(True)

        mvs = [IMP.core.BallMover(m, [p], 5) for p in [p1,p2]]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        initial=sf.evaluate(False)
        after = mc.optimize(num_mc_steps)

    def test_moved_score(self):
        """Test ISD restraints with move-aware MC scoring is identical"""

        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(20)]
        m1, mc1 = setup_system(coords)
        m2, mc2 = setup_system(coords)

        # Same seeed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)

    def test_moved_score_optimization(self):
        """Test ISD restraints with move-aware MC score decreases"""

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
        sf = IMP.core.RestraintsScoringFunction(restraints)
        mc.set_scoring_function(sf)
        mc.set_score_moved(True)

        mvs = [IMP.core.BallMover(m, [p], 5) for p in ps]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        for i in range(10):
            mc.optimize(len(ps))
            print(sf.evaluate(False))

    def test_moved_score_rigid_bodies(self):
        """Test ISD restraints with score-aware MC score decreases"""

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
        sf = IMP.core.RestraintsScoringFunction(list(set(restraints)))
        mc.set_scoring_function(sf)
        mc.set_score_moved(True)

        mvs = [IMP.core.RigidBodyMover(m, rb1, 1.0, 0.03),
               IMP.core.RigidBodyMover(m, rb2, 1.0, 0.03)]
        sm = IMP.core.SerialMover(mvs)
        mc.add_mover(sm)

        for i in range(10):
            mc.optimize(len(mvs))
            print(sf.evaluate(False))


if __name__ == '__main__':
    IMP.test.main()
