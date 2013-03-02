import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_it(self):
        """Test bond decorator pair container"""
        m= IMP.Model()
        p0=IMP.Particle(m)
        p1= IMP.Particle(m)
        p2= IMP.Particle(m)
        p3= IMP.Particle(m)
        b0= IMP.atom.Bonded.setup_particle(p0)
        b1= IMP.atom.Bonded.setup_particle(p1)
        b3= IMP.atom.Bonded.setup_particle(p3)
        b= IMP.atom.create_custom_bond(b0, b1, 1,1)

        pc= IMP.atom.BondedPairFilter()
        print pc.get_value((p0,p0))
        self.assertTrue(pc.get_value((p0,p1)))
        print 0
        self.assertFalse(pc.get_value((p2,p0)))
        print 1
        self.assertFalse(pc.get_value((p2,p2)))
        print 2
        self.assertFalse(pc.get_value((p0,p0)))
        self.assertFalse(pc.get_value((p0,p3)))


if __name__ == '__main__':
    IMP.test.main()
