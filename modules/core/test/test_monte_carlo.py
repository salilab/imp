import IMP
import IMP.test
import IMP.core
import IMP.container
import pickle
try:
    import jax
    import jax.random
    import jax.numpy as jnp
except ImportError:
    jax = None


class JAXOptimizerState(IMP.OptimizerState):
    def __init__(self, m, name):
        super().__init__(m, name)

    def _get_jax(self, state_index):
        import IMP._jax_util

        def init_func(ms):
            ms.optimizer_states[state_index] = {'calls': 0}
            return ms

        def apply_func(ms):
            ms.optimizer_states[state_index]['calls'] += 1
            return ms

        return IMP._jax_util.JAXOptimizerStateInfo(self, init_func, apply_func)


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
        rs.set_weight(0.7)
    elif use_container == 'singleton':
        sps = IMP.core.DistanceToSingletonScore(
                IMP.core.Harmonic(0, 1), (0, 0, 0))
        singles = [item for subl in pairs for item in subl]
        lsc = IMP.container.ListSingletonContainer(m, singles)
        rs = IMP.container.SingletonsRestraint(sps, lsc)
        rs.set_weight(0.8)
    else:
        prs = [IMP.core.PairRestraint(m, hps, p) for p in pairs]
        prs[0].set_weight(0.1)
        prs[1].set_weight(0.2)
        # Split restraints between two RestraintSets
        rs1 = IMP.RestraintSet(m)
        rs1.set_weight(2.3)
        rs1.add_restraints(prs[:2])
        rs2 = IMP.RestraintSet(m)
        rs2.set_weight(1.4)
        rs2.add_restraints(prs[2:])
        # Test scoring of nested RestraintSets
        rs = IMP.RestraintSet(m)
        rs.add_restraints([rs1, rs2])
        rs.set_weight(0.8)
    mc.set_scoring_function(rs)
    ms = [IMP.core.BallMover(m, x, 0.05) for x in ps[:5]]
    ms.append(IMP.core.BallMover(m, ps[5:8], 0.05))
    # No restraints on ps[9], so this should not change the score
    ms.append(IMP.core.BallMover(m, ps[9], 0.05))
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
    prs[0].set_weight(0.1)
    prs[1].set_weight(0.2)
    rs = IMP.RestraintSet(m)
    rs.set_weight(0.9)
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


def _setup_jax_mc(use_serial_mover=False):
    m = IMP.Model()
    mc = IMP.core.MonteCarlo(m)
    ds = []
    for i in range(2):
        p = IMP.Particle(m)
        d = IMP.core.XYZR.setup_particle(p)
        d.set_radius(.1)
        d.set_coordinates_are_optimized(True)
        ds.append(d)
    ds[1].set_coordinates(IMP.algebra.Vector3D(1., 2., 3.))
    hps = IMP.core.HarmonicDistancePairScore(0, 100)
    r = IMP.core.PairRestraint(m, hps, ds)
    rs = IMP.core.RestraintsScoringFunction([r])
    mc.set_scoring_function(rs)
    bm1 = IMP.core.BallMover(m, ds[0], 0.01)
    if use_serial_mover:
        bm2 = IMP.core.BallMover(m, ds[1], 0.01)
        sm = IMP.core.SerialMover([bm1, bm2])
        mc.add_mover(sm)
    else:
        mc.add_mover(bm1)
    mc.set_kt(0.01)
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
        mc.optimize(100)
        # SerialMover steps should match the entire MC
        self.assertEqual(mc.get_number_of_proposed_steps(), 100)
        self.assertEqual(mc.get_number_of_proposed_steps(),
                         mv.get_number_of_proposed())
        self.assertEqual(mc.get_number_of_accepted_steps(),
                         mv.get_number_of_accepted())
        # BallMover totals should match SerialMover
        for bm in ms:
            self.assertEqual(bm.get_number_of_proposed(), 10)
        self.assertEqual(sum(bm.get_number_of_accepted() for bm in ms),
                         mv.get_number_of_accepted())
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

        # Same seed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)

    def test_pickle(self):
        """Test (un-)pickle of MonteCarlo"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m, mc = setup_system(coords, False)
        mc.set_name("foo")
        mc.set_kt(1.)
        mc.set_last_accepted_energy(42.0)
        mc.set_number_of_downward_steps(19)
        mc.set_number_of_upward_steps(22)
        mc.set_number_of_rejected_steps(33)
        mc.set_score_threshold(20.0)
        mc.set_maximum_difference(10.0)
        dump = pickle.dumps(mc)
        newmc = pickle.loads(dump)
        self.assertEqual(newmc.get_name(), "foo")
        self.assertAlmostEqual(newmc.get_kt(), 1., delta=1e-4)
        self.assertAlmostEqual(newmc.get_last_accepted_energy(), 42.0,
                               delta=1e-4)
        self.assertEqual(newmc.get_number_of_downward_steps(), 19)
        self.assertEqual(newmc.get_number_of_upward_steps(), 22)
        self.assertEqual(newmc.get_number_of_proposed_steps(), 19 + 22 + 33)
        self.assertAlmostEqual(newmc.get_score_threshold(), 20.0, delta=1e-4)
        self.assertAlmostEqual(newmc.get_maximum_difference(),
                               10.0, delta=1e-4)

    def test_restart(self):
        """Test restart of MonteCarlo"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(30)]
        m, mc = setup_system(coords, False)
        mc.set_kt(1.)

        score1 = mc.optimize(100)

        # Save state of MC and random number generator
        rstate = IMP.random_number_generator.get_state()
        dump = pickle.dumps((m, mc, rstate))

        score2 = mc.optimize(100)

        # Restore state of MC and random number generator
        newm, newmc, newrstate = pickle.loads(dump)
        IMP.random_number_generator.set_state(rstate)
        score3 = newmc.optimize(100)

        self.assertGreater(abs(score1 - score2), 0.5)
        self.assertAlmostEqual(score2, score3, delta=1e-4)

    def test_pair_container_moved_same_trajectory(self):
        """MonteCarlo trajectory should not be changed by set_score_moved()
           when using ListPairContainer"""
        bb = IMP.algebra.get_unit_bounding_box_3d()
        coords = [IMP.algebra.get_random_vector_in(bb) for _ in range(10)]
        m1, mc1 = setup_system(coords, use_container='pair')
        m2, mc2 = setup_system(coords, use_container='pair')

        # Same seed, same system, so we should get identical trajectories
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

        # Same seed, same system, so we should get identical trajectories
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

        # Same seed, same system, so we should get identical trajectories
        IMP.random_number_generator.seed(99)
        mc1_score = mc1.optimize(100)

        mc2.set_score_moved(True)
        IMP.random_number_generator.seed(99)
        mc2_score = mc2.optimize(100)

        self.assertAlmostEqual(mc1_score, mc2_score, delta=1e-2)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_low_level(self):
        """Test low-level JAX implementation of MonteCarlo"""
        m, mc = _setup_jax_mc()
        # Initialize, get score of starting configuration
        ji = mc._get_jax()
        X = ji.get_jax_model()
        f = jax.jit(ji.init_func)
        mc_state = f(X, key=jax.random.key(42))

        # Create JAX function to run 2000 steps of MC
        j = jax.jit(
            lambda X: jax.lax.fori_loop(0, 2000,
                                        lambda i, X: ji.apply_func(X), X))
        mc_state = j(mc_state)
        # Check MC stats
        self.assertEqual(mc_state.rejected_steps
                         + mc_state.downward_steps_taken
                         + mc_state.upward_steps_taken, 2000)
        self.assertEqual(mc_state.rejected_steps
                         + mc_state.accepted_steps, 2000)
        # Particles should now be close
        new_jm = mc_state.jm
        self.assertLess(jnp.linalg.norm(new_jm["xyz"][1] - new_jm["xyz"][0]),
                        0.5)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_high_level(self):
        """Test high-level JAX implementation of MonteCarlo"""
        m, mc = _setup_jax_mc(use_serial_mover=True)
        mc.set_return_best(True)
        mc._optimize_jax(2000)

        # Check MC stats
        self.assertEqual(mc.get_number_of_proposed_steps(), 2000)
        self.assertLessEqual(mc.get_number_of_downward_steps()
                             + mc.get_number_of_upward_steps(), 2000)
        # Check SerialMover stats; should match the MC object
        sm, = mc.movers
        self.assertEqual(sm.get_number_of_proposed(),
                         mc.get_number_of_proposed_steps())
        self.assertEqual(sm.get_number_of_accepted(),
                         mc.get_number_of_accepted_steps())
        # Check BallMover stats
        bm1, bm2 = sm.get_derived_object().get_movers()
        self.assertEqual(bm1.get_number_of_proposed(), 1000)
        self.assertEqual(bm2.get_number_of_proposed(), 1000)
        self.assertEqual(bm1.get_number_of_accepted()
                         + bm2.get_number_of_accepted(),
                         sm.get_number_of_accepted())
        # Particles should now be close
        d0 = IMP.core.XYZ(m.get_particle(IMP.ParticleIndex(0)))
        d1 = IMP.core.XYZ(m.get_particle(IMP.ParticleIndex(1)))
        self.assertLess(
            IMP.algebra.get_distance(d0.get_coordinates(),
                                     d1.get_coordinates()), 0.5)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_jax_optimizer_state(self):
        """Test pure JAX OptimizerState"""
        def make_mc():
            m, mc = _setup_jax_mc()
            state1 = JAXOptimizerState(m, name="State1")
            mc.add_optimizer_state(state1)
            state2 = JAXOptimizerState(m, name="State2")
            state2.set_period(2)
            mc.add_optimizer_state(state2)
            return m, mc

        # Low level
        m, mc = make_mc()
        ji = mc._get_jax()
        X = ji.get_jax_model()
        f = jax.jit(ji.init_func)
        mc_state = f(X, key=jax.random.key(42))
        j = jax.jit(
            lambda X: jax.lax.fori_loop(0, 2000,
                                        lambda i, X: ji.apply_func(X), X))
        mc_state = j(mc_state)
        self.assertEqual(len(mc_state.optimizer_states), 2)
        self.assertEqual(mc_state.accepted_steps,
                         mc_state.optimizer_states[0]['calls'])
        self.assertEqual(mc_state.accepted_steps // 2,
                         mc_state.optimizer_states[1]['calls'])

        # High level
        m, mc = make_mc()
        mc._optimize_jax(2)


if __name__ == '__main__':
    IMP.test.main()
