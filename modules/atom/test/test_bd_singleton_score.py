import IMP
import IMP.test
import IMP.core
import IMP.algebra
import IMP.atom
import random

class Tests(IMP.test.TestCase):
    """Test the symmetry restraint"""
    def test_bd_ss(self):
        """Test the bond decorator score"""
        m= IMP.Model()
        p0= IMP.Particle(m)
        p1= IMP.Particle(m)
        d0= IMP.core.XYZ.setup_particle(p0)
        d1= IMP.core.XYZ.setup_particle(p1)
        b0= IMP.atom.Bonded.setup_particle(p0)
        b1= IMP.atom.Bonded.setup_particle(p1)
        b= IMP.atom.create_custom_bond(b0, b1, 3,2)
        ss= IMP.atom.BondSingletonScore(IMP.core.Linear(0,1))

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.Vector3D(0,0,3))
        self.assertEqual(ss.evaluate(b.get_particle(), None), 0)

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.Vector3D(0,0,4))
        self.assertEqual(ss.evaluate(b.get_particle(), None), 2)

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.Vector3D(0,0,2))
        self.assertEqual(ss.evaluate(b.get_particle(), None), -2)

    def test_bd_deriv(self):
        """Test derivatives of the bond SingletonScore"""
        m= IMP.Model()
        p0= IMP.Particle(m)
        p1= IMP.Particle(m)
        d0= IMP.core.XYZ.setup_particle(p0)
        d1= IMP.core.XYZ.setup_particle(p1)
        b0= IMP.atom.Bonded.setup_particle(p0)
        b1= IMP.atom.Bonded.setup_particle(p1)
        b= IMP.atom.create_custom_bond(b0, b1, 1.0, 20.0)
        ss= IMP.atom.BondSingletonScore(IMP.core.Harmonic(0,1))
        r = IMP.core.SingletonRestraint(ss, b.get_particle())
        m.add_restraint(r)

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(
           IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d()) \
           * (random.random() * 4.0 + 2.0))

        self.assertXYZDerivativesInTolerance(m, d0, 2.0, 5.0)
        self.assertXYZDerivativesInTolerance(m, d1, 2.0, 5.0)

if __name__ == '__main__':
    IMP.test.main()
