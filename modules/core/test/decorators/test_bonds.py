import unittest
import IMP
import IMP.test
import IMP.core

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Check bonds """
        m = IMP.Model()
        pa=IMP.Particle(m)
        pb=IMP.Particle(m)
        da= IMP.core.BondedDecorator.create(pa)
        db= IMP.core.BondedDecorator.create(pb)
        self.assertEqual(da.get_number_of_bonds(), 0,
                         "Wrong number of bonds on initialization")
        IMP.core.bond(da, db, IMP.core.BondDecorator.COVALENT)
        self.assertEqual(da.get_number_of_bonds(), 1,
                         "Expected to find a bond")
        self.assertEqual(da.get_number_of_bonds(), 1,
                         "Expected to find a bond")
        b= da.get_bond(0)
        self.assertEqual(b, db.get_bond(0), "Not same bond object")
        self.assertEqual(b.get_type(), IMP.core.BondDecorator.COVALENT,
                         "Wrong bond type")
        IMP.core.unbond(b)
        self.assertEqual(da.get_number_of_bonds(), 0,
                         "Expected not to find a bond")
        self.assertEqual(da.get_number_of_bonds(), 0,
                         "Expected not to find a bond")
        for p in m.get_particles():
            self.assertRaises(ValueError, IMP.core.BondDecorator.cast, p)



if __name__ == '__main__':
    unittest.main()
