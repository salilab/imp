import unittest
import IMP.utils
import IMP.test, IMP
import IMP.core
import IMP.algebra

class DistanceTests(IMP.test.TestCase):
    """Test the symmetry restraint"""
    def test_bd_ss(self):
        """Test the bond decorator score"""
        m= IMP.Model()
        p0= IMP.Particle(m)
        p1= IMP.Particle(m)
        d0= IMP.core.XYZDecorator.create(p0)
        d1= IMP.core.XYZDecorator.create(p1)
        b0= IMP.core.BondedDecorator.create(p0)
        b1= IMP.core.BondedDecorator.create(p1)
        b= IMP.core.custom_bond(b0, b1, 3,2)
        ss= IMP.core.BondSingletonScore(IMP.core.Linear(0,1))

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.Vector3D(0,0,3))
        self.assertEqual(ss.evaluate(b.get_particle(), None), 0)

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.Vector3D(0,0,4))
        self.assertEqual(ss.evaluate(b.get_particle(), None), 2)

        d0.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        d1.set_coordinates(IMP.algebra.Vector3D(0,0,2))
        self.assertEqual(ss.evaluate(b.get_particle(), None), -2)

if __name__ == '__main__':
    unittest.main()
