from __future__ import print_function
import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_mass(self):
        """Check Mass decorator"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        p2 = IMP.Particle(m)
        # Set up using mass value
        m1 = IMP.atom.Mass.setup_particle(p1, 1.0)
        self.assertAlmostEqual(m1.get_mass(), 1.0, delta=1e-4)
        m1.set_mass(2.0)
        self.assertAlmostEqual(m1.get_mass(), 2.0, delta=1e-4)
        # Set up using other particle
        m2 = IMP.atom.Mass.setup_particle(p2, m1)
        self.assertAlmostEqual(m2.get_mass(), 2.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
