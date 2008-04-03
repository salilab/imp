import unittest
import IMP
import IMP.test

masskey = IMP.FloatKey("mass")

class TestGravityCenter(IMP.test.TestCase):
    """Tests for GravityCenterScoreState"""

    def _make_particles(self, model, x, y, z, weighted):
        """Make a group of particles with center at x, y, z"""
        ps = IMP.Particles()
        if weighted:
            for (dx,dy,dz,wt) in ((-2,0,0,1), (1,0,0,2), (0,-1,0,2), (0,2,0,1),
                                  (0,0,-3,1), (0,0,1,3)):
                p = self.create_point_particle(model, x+dx, y+dy, z+dz)
                p.add_attribute(masskey, wt, False)
                ps.append(p)
        else:
            for (dx,dy,dz) in ((-1,0,0), (1,0,0), (0,-1,0), (0,1,0),
                               (0,0,-1), (0,0,1)):
                ps.append(self.create_point_particle(model, x+dx, y+dy, z+dz))
        return ps

    def _make_gravity_center(self, model, x, y, z, weighted):
        """Make a gravity center of particles at x, y, z"""
        ps = self._make_particles(model, x, y, z, weighted)
        center = self.create_point_particle(model, 0, 0, 0)
        if weighted:
            gc = IMP.GravityCenterScoreState(center, masskey, ps)
        else:
            gc = IMP.GravityCenterScoreState(center, IMP.FloatKey(), ps)
        model.add_score_state(gc)
        return ps, center, gc

    def test_set_position(self):
        """Make sure that gravity center coordinates are correct"""
        model = IMP.Model()
        for weighted in (False, True):
            (ps, center, gc) = self._make_gravity_center(model, 10.0, 20.0,
                                                         30.0, weighted)
            gc.update()
            # The gravity center's xyz should not be optimizable:
            xkey = IMP.FloatKey("x")
            ykey = IMP.FloatKey("y")
            zkey = IMP.FloatKey("z")
            self.assertEqual(center.get_is_optimized(xkey), False)
            self.assertEqual(center.get_is_optimized(ykey), False)
            self.assertEqual(center.get_is_optimized(zkey), False)
            # Coordinates should match the center of the original particles:
            self.assertEqual(center.get_value(xkey), 10.0)
            self.assertEqual(center.get_value(ykey), 20.0)
            self.assertEqual(center.get_value(zkey), 30.0)

    def test_optimize(self):
        """Optimizer should bring gravity centers together"""
        model = IMP.Model()
        (ps1, cen1, gc1) = self._make_gravity_center(model, 10.0, 20.0, 30.0,
                                                     False)
        (ps2, cen2, gc2) = self._make_gravity_center(model, 0.0, 0.0, 0.0,
                                                     True)
        r = IMP.DistanceRestraint(IMP.Harmonic(0.2, 7000.0), cen1, cen2)
        model.add_restraint(r)
        opt = IMP.ConjugateGradients()
        opt.set_model(model)
        opt.set_threshold(1e-7)
        e = opt.optimize(100)
        dist = self.particle_distance(cen1, cen2)
        self.assertInTolerance(0.2, dist, 1e-3)

if __name__ == '__main__':
    unittest.main()
