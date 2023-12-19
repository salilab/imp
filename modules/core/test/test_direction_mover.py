import IMP
import IMP.algebra
import IMP.core
import IMP.atom
import IMP.test
import pickle


class Tests(IMP.test.TestCase):

    """Tests for DirectionMover."""

    def test_init(self):
        """Test creation of surface mover."""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d.set_direction_is_optimized(True)
        mv = IMP.core.DirectionMover(d, .1, 1.)
        mv.set_was_used(True)

    def test_propose_move(self):
        """Test proposing move alters center and direction."""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d0 = d.get_direction()
        d.set_direction_is_optimized(True)
        mv = IMP.core.DirectionMover(d, .1, 1.)
        mv.propose()
        self.assertNotAlmostEqual((d0 - d.get_direction()).get_magnitude(), 0)

    def test_propose_reflect(self):
        """Test reflect correctly flips direction."""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d0 = d.get_direction()
        d.set_direction_is_optimized(True)
        mv = IMP.core.DirectionMover(d, 0, 1.)
        mv.propose()
        self.assertAlmostEqual((d0 + d.get_direction()).get_magnitude(), 0)

    def test_reject_restores_initial_state(self):
        """Test rejecting a move returns the surface to previous state."""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d0 = d.get_direction()
        d.set_direction_is_optimized(True)
        mv = IMP.core.DirectionMover(d, .1, 1.)
        mv.propose()
        mv.reject()
        self.assertAlmostEqual((d0 - d.get_direction()).get_magnitude(), 0)

    def test_inputs(self):
        """Test only input is Direction."""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d.set_direction_is_optimized(True)
        mv = IMP.core.DirectionMover(d, .1, 1.)
        self.assertSetEqual(set([d.get_particle()]), set(mv.get_inputs()))
        mv.set_was_used(True)

    def test_pickle(self):
        """Test (un-)pickle of DirectionMover"""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d.set_direction_is_optimized(True)
        mvr = IMP.core.DirectionMover(d, .1, 1.)
        mvr.set_name("foo")
        dump = pickle.dumps(mvr)

        newmvr = pickle.loads(dump)
        self.assertEqual(newmvr.get_name(), "foo")
        self.assertSetEqual(set([d.get_particle()]), set(newmvr.get_inputs()))

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of DirectionMover via polymorphic pointer"""
        m = IMP.Model()
        d = IMP.core.Direction.setup_particle(IMP.Particle(m),
                                              IMP.algebra.Vector3D(0, 0, 1))
        d.set_direction_is_optimized(True)
        mvr = IMP.core.DirectionMover(d, .1, 1.)
        mvr.set_name("foo")
        sm = IMP.core.SerialMover([mvr])
        dump = pickle.dumps(sm)

        newsm = pickle.loads(dump)
        newmvr, = newsm.get_movers()
        self.assertEqual(newmvr.get_name(), "foo")


if __name__ == '__main__':
    IMP.test.main()
