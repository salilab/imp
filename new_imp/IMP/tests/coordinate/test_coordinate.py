import unittest
import os
import IMP
import IMP.test
import IMP.utils

class test_coordinate(IMP.test.IMPTestCase):
    """Test various absolute position restraints"""

    def setUp(self):
        self.imp_model = IMP.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        for p in range(12):
            self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                        0., 0., 0.))
        self.opt = IMP.ConjugateGradients()


    def _do_test_min(self, coord, mask):
        """All coordinate values should be greater than the set minimum"""
        self.randomize_particles(self.particles, 50.0)

        rs = IMP.RestraintSet("%s-min" % coord)
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_lower_bound",
                                                     8.0, 0.1)
        while p_iter.next():
            self.rsrs.append(IMP.CoordinateRestraint(self.imp_model,
                                                     p_iter.get(),
                                                     "%s_AXIS" % coord.upper(),
                                                     score_func_params))
            r = self.rsrs[len(self.rsrs)-1]
            rs.add_restraint(r)

        self.opt.optimize(self.imp_model, 55, 1e-5)
        for p in self.particles:
            self.assert_(self.check_abs_pos(p, '>=', 7.999, *mask),
                         "%s-min condition" % coord)
        rs.set_is_active(False)

    def test_min(self):
        """All coordinate values should be greater than the set minimum"""
        for (coord, mask) in (('x', (1,0,0)), ('y', (0,1,0)), ('z', (0,0,1))):
            self._do_test_min(coord, mask)

    def _do_test_max(self, coord, mask):
        """All coordinate values should be less than the set maximum"""
        self.randomize_particles(self.particles, 50.0)

        rs = IMP.RestraintSet("%s-max" % coord)
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     -8.0, 0.1)
        while p_iter.next():
            self.rsrs.append(IMP.CoordinateRestraint(self.imp_model,
                                                     p_iter.get(),
                                                     "%s_AXIS" % coord.upper(),
                                                     score_func_params))
            r = self.rsrs[len(self.rsrs)-1]
            rs.add_restraint(r)

        self.opt.optimize(self.imp_model, 55, 1e-5)
        for p in self.particles:
            self.assert_(self.check_abs_pos(p, '<=', -7.9999, *mask),
                         "%s-max condition" % coord)
        rs.set_is_active(False)

    def test_max(self):
        """All coordinate values should be less than the set maximum"""
        for (coord, mask) in (('x', (1,0,0)), ('y', (0,1,0)), ('z', (0,0,1))):
            self._do_test_max(coord, mask)

    def test_xy_radial(self):
        """All xy distances should be less than the set maximum"""
        self.randomize_particles(self.particles, 50.0)

        rs = IMP.RestraintSet("xy_radial")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     8.0, 0.1)
        while p_iter.next():
            r = IMP.CoordinateRestraint(self.imp_model, p_iter.get(),
                                        "XY_RADIAL", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        self.opt.optimize(self.imp_model, 55, 1e-5)
        for p in self.particles:
            self.assert_(self.check_abs_pos(p, '<=', 8.001, 1, 1, 0),
                         "xy_radial condition")
        rs.set_is_active(False)

    def test_xz_radial(self):
        """All xz distances should be less than the set maximum"""
        self.randomize_particles(self.particles, 50.0)

        rs = IMP.RestraintSet("xz_radial")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     8.0, 0.1)
        while p_iter.next():
            r = IMP.CoordinateRestraint(self.imp_model, p_iter.get(),
                                        "XZ_RADIAL", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        self.opt.optimize(self.imp_model, 55, 1e-5)
        for p in self.particles:
            self.assert_(self.check_abs_pos(p, '<=', 8.001, 1, 0, 1),
                         "xz_radial condition")
        rs.set_is_active(False)

    def test_yz_radial(self):
        """All yz distances should be less than the set maximum"""
        self.randomize_particles(self.particles, 50.0)

        self.restraint_sets.append(IMP.RestraintSet("yz_radial"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     8.0, 0.1)
        while p_iter.next():
            r = IMP.CoordinateRestraint(self.imp_model, p_iter.get(),
                                        "YZ_RADIAL", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        self.opt.optimize(self.imp_model, 55, 1e-5)
        for p in self.particles:
            self.assert_(self.check_abs_pos(p, '<=', 8.001, 0, 1, 1),
                         "yz_radial condition")
        rs.set_is_active(False)

    def test_xyz_sphere(self):
        """All xyz distances should be less than the set maximum"""
        self.randomize_particles(self.particles, 50.0)

        self.restraint_sets.append(IMP.RestraintSet("xyz_sphere"))
        rs = self.restraint_sets[len(self.restraint_sets)-1]
        self.imp_model.add_restraint_set(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     8.0, 0.1)
        while p_iter.next():
            r = IMP.CoordinateRestraint(self.imp_model, p_iter.get(),
                                        "XYZ_SPHERE", score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        self.opt.optimize(self.imp_model, 55, 1e-5)
        for p in self.particles:
            self.assert_(self.check_abs_pos(p, '<=', 8.001, 1, 1, 1),
                         "xyz_sphere condition")
        rs.set_is_active(False)

if __name__ == '__main__':
    unittest.main()
