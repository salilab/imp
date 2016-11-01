import IMP
import IMP.algebra
import IMP.core
import IMP.atom
import IMP.test


class Tests(IMP.test.TestCase):

    """Tests for SurfaceMover."""

    def test_init(self):
        """Test creation of surface mover."""
        m = IMP.Model()
        surf = IMP.core.Surface.setup_particle(IMP.Particle(m))
        surf.set_coordinates_are_optimized(True)
        surf.set_normal_is_optimized(True)
        mv = IMP.core.SurfaceMover(surf, 1, .1, 1.)
        mv.set_was_used(True)

    def test_propose_move(self):
        """Test proposing move alters center and normal."""
        m = IMP.Model()
        surf = IMP.core.Surface.setup_particle(IMP.Particle(m))
        n = surf.get_normal()
        c = surf.get_coordinates()
        surf.set_coordinates_are_optimized(True)
        surf.set_normal_is_optimized(True)
        mv = IMP.core.SurfaceMover(surf, 1, .1, 1.)
        mv.propose()
        self.assertNotAlmostEqual((n - surf.get_normal()).get_magnitude(), 0)
        self.assertNotAlmostEqual((c - surf.get_coordinates()).get_magnitude(), 0)

    def test_propose_reflect(self):
        """Test reflect correctly flips normal."""
        m = IMP.Model()
        surf = IMP.core.Surface.setup_particle(IMP.Particle(m))
        n = surf.get_normal()
        surf.set_normal_is_optimized(True)
        mv = IMP.core.SurfaceMover(surf, 0, 0, 1.)
        mv.propose()
        self.assertAlmostEqual((n + surf.get_normal()).get_magnitude(), 0)

    def test_reject_restores_initial_state(self):
        """Test rejecting a move returns the surface to previous state."""
        m = IMP.Model()
        surf = IMP.core.Surface.setup_particle(IMP.Particle(m))
        n = surf.get_normal()
        c = surf.get_coordinates()
        surf.set_coordinates_are_optimized(True)
        surf.set_normal_is_optimized(True)
        mv = IMP.core.SurfaceMover(surf, 1, .1, 1.)
        mv.propose()
        mv.reject()
        self.assertAlmostEqual((n - surf.get_normal()).get_magnitude(), 0)
        self.assertAlmostEqual((c - surf.get_coordinates()).get_magnitude(), 0)

    def test_inputs(self):
        """Test only input is Surface."""
        m = IMP.Model()
        surf = IMP.core.Surface.setup_particle(IMP.Particle(m))
        surf.set_coordinates_are_optimized(True)
        surf.set_normal_is_optimized(True)
        mv = IMP.core.SurfaceMover(surf, 1, .1, 1.)
        self.assertSetEqual(set([surf.get_particle()]), set(mv.get_inputs()))
        mv.set_was_used(True)


if __name__ == '__main__':
    IMP.test.main()
