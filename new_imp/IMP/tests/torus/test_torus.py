import unittest
import os
import IMP
import IMP.test
import IMP.utils

class test_torus(IMP.test.IMPTestCase):
    """Class to test torus restraints"""

    def setUp(self):
        """Build test model and optimizer"""
        self.imp_model = IMP.Model()
        self.particles = []
        self.restraint_sets = []
        self.rsrs = []

        for p in range(12):
            self.particles.append(IMP.utils.XYZParticle(self.imp_model,
                                                        0., 0., 0.))
        self.opt = IMP.ConjugateGradients()


    def test_torus(self):
        """All particles should be inside the specified torus"""
        rs = IMP.RestraintSet("torus")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint(rs)

        p_iter = IMP.ParticleIterator()
        p_iter.reset(self.imp_model)
        score_func_params = IMP.BasicScoreFuncParams("harmonic_upper_bound",
                                                     0.0, 0.1)
        while p_iter.next():
            r = IMP.TorusRestraint(self.imp_model, p_iter.get(), 50, 10,
                                   score_func_params)
            self.rsrs.append(r)
            rs.add_restraint(r)

        # Randomize particle coordinates
        self.randomize_particles(self.particles, 50.0)

        self.opt.optimize(self.imp_model, 50, 1e-4)
        for p in self.particles:
            coord = (p.x(), p.y(), p.z())
            self.assert_(self.check_in_torus(coord, 50, 10.001),
                         "in torus condition")

if __name__ == '__main__':
    unittest.main()
