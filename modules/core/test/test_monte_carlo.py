import IMP
import IMP.test
import IMP.core
import IMP.container


def setup_system(coords, use_container):
    m = IMP.Model()
    mc = IMP.core.MonteCarlo(m)
    ps = []
    for i in range(len(coords)):
        p = IMP.Particle(m)
        d = IMP.core.XYZR.setup_particle(p)
        ps.append(d)
        d.set_coordinates(coords[i])
        d.set_radius(.1)
        d.set_coordinates_are_optimized(True)
    # Make the scoring function dependent on at least one ScoreState,
    # so we can check that any needed states are updated
    cent = IMP.core.Centroid.setup_particle(IMP.Particle(m), [ps[5], ps[6]])
    hps = IMP.core.HarmonicDistancePairScore(1, 100)
    pairs = ((ps[0], ps[1]), (ps[1], ps[2]), (ps[2], ps[3]), (ps[4], cent))
    if use_container == 'pair':
        lpc = IMP.container.ListPairContainer(m, pairs)
        rs = IMP.container.PairsRestraint(hps, lpc)
    elif use_container == 'singleton':
        sps = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), (0, 0, 0))
        singles = [item for subl in pairs for item in subl]
        lsc = IMP.container.ListSingletonContainer(m, singles)
        rs = IMP.container.SingletonsRestraint(sps, lsc)
    else:
        prs = [IMP.core.PairRestraint(m, hps, p) for p in pairs]
        rs = IMP.RestraintSet(m)
        rs.add_restraints(prs)
    mc.set_scoring_function(rs)
    ms = [IMP.core.BallMover(m, x, 0.05) for x in ps[:5]]
    ms.append(IMP.core.BallMover(m, ps[5:8], 0.05))
    mv = IMP.core.SerialMover(ms)
    mc.add_mover(mv)
    # Always reject upwards moves
    mc.set_kt(0.)
    mc.set_return_best(False)
    return m, mc


def setup_rigid_body_system(coords):
    # Similar to setup_system, but group the particles into rigid bodies
    m = IMP.Model()
    mc = IMP.core.MonteCarlo(m)
    ps = []
    for i in range(len(coords)):
        p = IMP.Particle(m)
        d = IMP.core.XYZR.setup_particle(p)
        ps.append(d)
        d.set_coordinates(coords[i])
        d.set_radius(.1)
    # Split system into two rigid bodies
    ps1, ps2 = ps[:len(ps)//2], ps[len(ps)//2:]
    rb1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps1)
    rb1.set_coordinates_are_optimized(True)
    rb2 = IMP.core.RigidBody.setup_particle(IMP.Particle(m), ps2)
    rb2.set_coordinates_are_optimized(True)

    # Create both inter- and intra-body distance restraints
    hps = IMP.core.HarmonicDistancePairScore(1, 100)
    pairs = ((ps1[0], ps2[0]), (ps1[1], ps2[1]), (ps1[2], ps1[3]))
    prs = [IMP.core.PairRestraint(m, hps, p) for p in pairs]
    rs = IMP.RestraintSet(m)
    rs.add_restraints(prs)
    mc.set_scoring_function(rs)

    # Also test with non-rigid members
    rb1.set_is_rigid_member(ps1[0], False)
    rb2.set_is_rigid_member(ps2[0], False)
    ps1[0].set_coordinates_are_optimized(True)
    ps2[0].set_coordinates_are_optimized(True)

    # Move the two rigid bodies and the two rigid members
    ms = [IMP.core.BallMover(m, x, 0.05) for x in (ps1[0], ps2[0])]
    ms.append(IMP.core.RigidBodyMover(m, rb1, 0.05, 0.05))
    ms.append(IMP.core.RigidBodyMover(m, rb2, 0.05, 0.05))
    mv = IMP.core.SerialMover(ms)
    mc.add_mover(mv)
    # Always reject upwards moves
    mc.set_kt(0.)
    mc.set_return_best(False)
    return m, mc


class Tests(IMP.test.TestCase):

    def test_stats(self):
        """Test MonteCarlo stats"""
        m = IMP.Model()
        IMP.set_log_level(IMP.WARNING)
        mc = IMP.core.MonteCarlo(m)
        mc.set_log_level(IMP.WARNING)
        ps = []
        bb = IMP.algebra.get_unit_bounding_box_3d()
        for i in range(0, 10):
            p = IMP.Particle(m)
            d = IMP.core.XYZR.setup_particle(p)
            ps.append(d)
            d.set_coordinates(IMP.algebra.get_random_vector_in(bb))
            d.set_radius(.1)
            d.set_coordinates_are_optimized(True)
        cpc = IMP.container.ConsecutivePairContainer(m, ps)
        hps = IMP.core.HarmonicDistancePairScore(1, 100)
        r = IMP.container.PairsRestraint(hps, cpc)
        mc.set_scoring_function([r])
        ms = [IMP.core.BallMover(m, x, .1) for x in ps]
        mv = IMP.core.SerialMover(ms)
        mc.add_mover(mv)
        self.assertEqual(mc.get_number_of_accepted_steps(), 0)
        self.assertEqual(mc.get_number_of_upward_steps(), 0)
        self.assertEqual(mc.get_number_of_downward_steps(), 0)
        self.assertEqual(mc.get_number_of_proposed_steps(), 0)
        for i in range(0, 10):
            mc.optimize(100)
            self.assertEqual(mc.get_number_of_accepted_steps(),
                             mc.get_number_of_upward_steps() +
                             mc.get_number_of_downward_steps())
            self.assertEqual(mc.get_number_of_proposed_steps(), 100)

    def test_restraint_set_moved_same_trajectory(self):
        """MonteCarlo trajectory should not be changed by set_score_moved()
           when using RestraintSet"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m1, mc1 = setup_system(coords, use_container=False)
        m2, mc2 = setup_system(coords, use_container=False)

        # Same seeed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)

    def test_pair_container_moved_same_trajectory(self):
        """MonteCarlo trajectory should not be changed by set_score_moved()
           when using ListPairContainer"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m1, mc1 = setup_system(coords, use_container='pair')
        m2, mc2 = setup_system(coords, use_container='pair')

        # Same seeed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)

    def test_singleton_container_moved_same_trajectory(self):
        """MonteCarlo trajectory should not be changed by set_score_moved()
           when using ListSingletonContainer"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m1, mc1 = setup_system(coords, use_container='singleton')
        m2, mc2 = setup_system(coords, use_container='singleton')

        # Same seeed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)

    def test_rigid_body_moved_same_trajectory(self):
        """MonteCarlo trajectory should not be changed by set_score_moved()
           when using rigid bodies"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m1, mc1 = setup_rigid_body_system(coords)
        m2, mc2 = setup_rigid_body_system(coords)

        # Same seeed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)


if __name__ == '__main__':
    IMP.test.main()
