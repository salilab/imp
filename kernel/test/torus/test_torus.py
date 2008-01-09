import unittest
import os
import IMP
import IMP.test
import IMP.utils

class TorusRestraintTests(IMP.test.TestCase):
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
        self.opt.set_model(self.imp_model);
        self.opt.set_threshold(1e-4)


    def test_torus(self):
        """All particles should be inside the specified torus"""
        rs = IMP.RestraintSet("torus")
        self.restraint_sets.append(rs)
        self.imp_model.add_restraint(rs)

        for p in self.imp_model.get_particles():
            r = IMP.TorusRestraint(p, 50, 10, IMP.HarmonicUpperBound(0.0, 0.1))
            self.rsrs.append(r)
            rs.add_restraint(r)

        # Randomize particle coordinates
        self.randomize_particles(self.particles, 50.0)

        self.opt.optimize(50)
        for p in self.particles:
            coord = (p.x(), p.y(), p.z())
            self.assert_(self.check_in_torus(coord, 50, 10.001),
                         "in torus condition")

if __name__ == '__main__':
    unittest.main()
