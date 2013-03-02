import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_bonded(self):
        """Check bonds """
        m = IMP.Model()
        pa=IMP.Particle(m)
        pb=IMP.Particle(m)
        da= IMP.atom.Bonded.setup_particle(pa)
        db= IMP.atom.Bonded.setup_particle(pb)
        self.assertEqual(da.get_number_of_bonds(), 0,
                         "Wrong number of bonds on initialization")
        IMP.atom.create_bond(da, db, IMP.atom.Bond.SINGLE)
        self.assertEqual(da.get_number_of_bonds(), 1,
                         "Expected to find a bond")
        self.assertEqual(da.get_number_of_bonds(), 1,
                         "Expected to find a bond")
        b= da.get_bond(0)
        self.assertEqual(b, db.get_bond(0), "Not same bond object")
        self.assertEqual(b.get_type(), IMP.atom.Bond.SINGLE,
                         "Wrong bond type")
        self.assertEqual(IMP.atom.get_bond(da, db), b)
        IMP.atom.destroy_bond(b)
        self.assertEqual(da.get_number_of_bonds(), 0,
                         "Expected not to find a bond")
        self.assertEqual(da.get_number_of_bonds(), 0,
                         "Expected not to find a bond")
        for p in m.get_particles():
            self.assertEqual(IMP.atom.Bond.decorate_particle(p),
                             IMP.atom.Bond())



if __name__ == '__main__':
    IMP.test.main()
