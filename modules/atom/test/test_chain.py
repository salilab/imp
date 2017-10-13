from __future__ import division
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display


class Tests(IMP.test.TestCase):

    def test_chain(self):
        """Test Chain decorator"""
        m = IMP.Model()
        p = IMP.Particle(m)
        self.assertFalse(IMP.atom.Chain.get_is_setup(p))
        c = IMP.atom.Chain.setup_particle(p, "A")
        self.assertTrue(IMP.atom.Chain.get_is_setup(p))
        self.assertEqual(c.get_id(), "A")
        c.set_id("B")
        self.assertEqual(c.get_id(), "B")
        self.assertEqual(c.get_sequence(), "")
        c.set_sequence("CCY")
        self.assertEqual(c.get_sequence(), "CCY")

if __name__ == '__main__':
    IMP.test.main()
