from __future__ import print_function
import IMP
import IMP.test
import IMP.atom
import IMP.core
import IMP.algebra
import math

class Tests(IMP.test.TestCase):

    def make_angle(self, m, angle):
        p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(-1,0,0))
        p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(0,0,0))
        p3 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(-math.cos(angle),
                                              math.sin(angle), 0))
        return (p1, p2, p3)

    def test_angle(self):
        """Test CAAngleRestraint"""
        m = IMP.Model()
        p1, p2, p3 = self.make_angle(m, 1.0)

        r = IMP.atom.CAAngleRestraint(p1, p2, p3, [0.8, 1.1, 1.3],
                                      [100., 200., 300.])
        self.assertAlmostEqual(r.evaluate(False), 200., delta=1e-4)

    def make_dihedral(self, m, angle1):
        # Put particles 2-3-4 in the xy plane
        p2 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(0,0,0))
        p3 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(0,1,0))
        p4 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(1,0,0))

        # Place p1 above at angle1 and p5 above at 90 degrees
        p1 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(math.cos(angle1), 0,
                                              math.sin(angle1)))
        p5 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                         IMP.algebra.Vector3D(1,0,1))
        return (p1, p2, p3, p4, p5)

    def test_dihedral(self):
        """Test CADihedralRestraint"""
        m = IMP.Model()
        p1, p2, p3, p4, p5 = self.make_dihedral(m, 0.5)

        r = IMP.atom.CADihedralRestraint(p1, p2, p3, p4, p5,
                   [0.3, 0.6, 0.8],
                   [-1.4, -1.6, -1.8],
                   [0, 0, 0,
                    0, 100, 0,
                    0, 0, 0])
        self.assertAlmostEqual(r.evaluate(False), 100., delta=1e-4)

if __name__ == '__main__':
    IMP.test.main()
