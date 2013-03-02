import IMP
import IMP.test
import IMP.core
import IMP.atom
import math

class Tests(IMP.test.TestCase):
    """Tests for bond refined"""

    def test_allp(self):
        """Check that BondRefiner works"""
        IMP.base.set_log_level(IMP.MEMORY)
        m=IMP.Model()
        p0= IMP.Particle(m)
        b0= IMP.atom.Bonded.setup_particle(p0)
        p1= IMP.Particle(m)
        b1= IMP.atom.Bonded.setup_particle(p1)
        b= IMP.atom.create_custom_bond(b0, b1, 1)
        print "creating r"
        r= IMP.atom.BondEndpointsRefiner()
        self.assertFalse(r.get_can_refine(p0))
        self.assertFalse(r.get_can_refine(p1))
        self.assertTrue(r.get_can_refine(b.get_particle()))
        print "get"
        ps= r.get_refined(b.get_particle())
        print "post get"
        self.assertEqual(ps[0], p0)
        self.assertEqual(ps[1], p1)
        print "clean r"
        r=None
        print "done clean r"


if __name__ == '__main__':
    IMP.test.main()
