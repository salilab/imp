import unittest
import IMP
import IMP.test
import IMP.core

rk = IMP.FloatKey("radius")

class TestBondCover(IMP.test.TestCase):
    """Tests for BondCover"""

    def test_it(self):
        """Test BondCoverSingletonModifier and PropagateBondDerivativesSM"""
        # return [model, particles, bonds]
        m= IMP.Model()
        p0= IMP.Particle()
        m.add_particle(p0)
        xyz0= IMP.core.XYZDecorator.create(p0)
        b0= IMP.core.BondedDecorator.create(p0)
        p1= IMP.Particle()
        m.add_particle(p1)
        xyz1= IMP.core.XYZDecorator.create(p1)
        b1= IMP.core.BondedDecorator.create(p1)
        b= IMP.core.custom_bond(b0, b1, 1)
        xyzrb= IMP.core.XYZRDecorator.create(b.get_particle())
        mod= IMP.core.CoverBondSingletonModifier()
        xyz0.set_coordinates(IMP.random_vector_in_unit_box())
        xyz1.set_coordinates(IMP.random_vector_in_unit_box())
        mod.apply(b.get_particle())
        center= (xyz0.get_coordinates() + xyz1.get_coordinates()) /2.0
        dr= center- xyz0.get_coordinates()
        r= (dr*dr) **.5
        print r
        print center
        xyz0.show()
        xyz1.show()
        xyzrb.show()
        self.assertInTolerance(r, xyzrb.get_radius(), r*.1,
                               "Radius is not close enough")
        self.assertInTolerance(center[0], xyzrb.get_x(), .1,
                                "X is not close enough")
        self.assertInTolerance(center[1], xyzrb.get_y(), .1,
                                "Y is not close enough")
        self.assertInTolerance(center[2], xyzrb.get_z(), .1,
                                "Z is not close enough")
        v= IMP.Vector3D(1,1,1)
        da= IMP.DerivativeAccumulator()
        xyzrb.add_to_coordinates_derivative(v, da)
        pd= IMP.core.PropagateBondDerivativesSingletonModifier()
        pd.apply(xyzrb.get_particle())
        xyz0.get_derivatives().show()
        xyz1.get_derivatives().show()
        for i in range(0,3):
            self.assertEqual(v[i]/2.0, xyz0.get_derivatives()[i])
            self.assertEqual(v[i]/2.0, xyz1.get_derivatives()[i])


if __name__ == '__main__':
    unittest.main()
