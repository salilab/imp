import IMP
import IMP.core
import IMP.algebra
import IMP.core
import IMP.container
import IMP.test


class Volume(IMP.test.TestCase):

    """Tests for angle restraints"""

    def test_volume_1(self):
        """Testing that volume restraint can separate balls"""
        if not hasattr(IMP.core, 'VolumeRestraint'):
            self.skipTest('VolumeRestraint not built (no CGAL)')
        m = IMP.Model()
        ps = []
        IMP.set_log_level(IMP.VERBOSE)
        for i in range(0, 3):
            p = IMP.Particle(m)
            v = IMP.algebra.get_random_vector_in(
                IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                                          IMP.algebra.Vector3D(5, 5, 5)))
            d = IMP.core.XYZR.setup_particle(p, IMP.algebra.Sphere3D(v, 4))
            ps.append(p)
            p.set_is_optimized(IMP.FloatKey("x"), True)
            p.set_is_optimized(IMP.FloatKey("y"), True)
            p.set_is_optimized(IMP.FloatKey("z"), True)
        sc = IMP.container.ListSingletonContainer(m, ps)
        vr = IMP.core.VolumeRestraint(
            IMP.core.Harmonic(0,
                              1),
            sc,
            4 ** 3 * 3.1415 * 4.0 / 3.0 * len(ps))
        sf = IMP.core.RestraintsScoringFunction([vr])
        mc = IMP.core.MonteCarlo(m)
        mc.set_scoring_function(sf)
        mc.add_mover(IMP.core.BallMover(m, ps, 4))
        mc.set_score_threshold(.2)
        for i in range(5):
            try:
                mc.optimize(10)
                if sf.evaluate(False) < .2:
                    break
            except IMP.ValueException:
                # Catch CG failure
                pass
        self.assertLess(sf.evaluate(False), .2)


if __name__ == '__main__':
    IMP.test.main()
