import unittest
import os
import IMP
import IMP.test
import IMP.utils

class CoordinateTests(IMP.test.IMPTestCase):
    """Test various absolute position restraints"""

    def setUp(self):
        self.model = IMP.Model()
        p= IMP.Particle()
        self.pi= self.model.add_particle(p);
        d= IMP.XYZDecorator.create(p)
        d.set_coordinates_are_optimized(True)
        pc= IMP.Particle()
        self.pic= self.model.add_particle(pc);
        dc= IMP.XYZDecorator.create(pc)

        self.opt = IMP.SteepestDescent()
        self.opt.set_model(self.model)
        #self.opt.set_threshold(1e-5)


    def _do_test(self, center, sf):
        """All coordinate values should be greater than the set minimum"""

        r= IMP.SphericalRestraint(self.model,
                                  self.model.get_particle(self.pi),
                                  center[0], center[1], center[2],
                                  sf)
        ri=self.model.add_restraint(r)

        self.opt.optimize(55)
        self.model.get_restraint(ri).set_is_active(False)

    def _get_center(self):
        v= IMP.Floats()
        d= IMP.XYZDecorator.cast(self.model.get_particle(self.pic))
        v.append(d.get_x())
        v.append(d.get_y())
        v.append(d.get_z())
        return v

    def test_in_ball(self):
        """Testing that restraint keeps point in ball"""
        self.randomize_particles(self.model.get_particles(), 50.0)
        f= IMP.HarmonicUpperBound(10,.1)
        c= self._get_center()
        self._do_test(c, f)
        pd= IMP.XYZDecorator.cast(self.model.get_particle(self.pi))
        cd= IMP.XYZDecorator.cast(self.model.get_particle(self.pic))
        d= IMP.distance(pd, cd)
        self.assert_(d < 11)

    def test_on_ball(self):
        """Testing that restraint keeps point on sphere"""
        self.randomize_particles(self.model.get_particles(), 50.0)
        f= IMP.Harmonic(10,.1)
        c= self._get_center()
        self._do_test( c, f)
        pd= IMP.XYZDecorator.cast(self.model.get_particle(self.pi))
        cd= IMP.XYZDecorator.cast(self.model.get_particle(self.pic))
        d= IMP.distance(pd, cd)
        self.assert_(d < 11)
        self.assert_(d > 9)


if __name__ == '__main__':
    unittest.main()
