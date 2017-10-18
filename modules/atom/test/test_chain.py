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

    def test_chain_type(self):
        """Test Chain type"""
        m = IMP.Model()
        p = IMP.Particle(m)
        c = IMP.atom.Chain.setup_particle(p, "A")
        t=c.get_chain_type()
        self.assertEqual(t,IMP.atom.UnknownChainType)


        for t in [IMP.atom.DPolypeptide,
                  IMP.atom.LPolypeptide,
                  IMP.atom.Polydeoxyribonucleotide,
                  IMP.atom.Polyribonucleotide,
                  IMP.atom.DPolysaccharide,
                  IMP.atom.LPolysaccharide]:
            c.set_chain_type(t)
            ttest=c.get_chain_type()
            self.assertEqual(ttest,t)

        c.set_chain_type(IMP.atom.Protein)
        t = c.get_chain_type()
        self.assertEqual(t,IMP.atom.LPolypeptide)

        c.set_chain_type(IMP.atom.DNA)
        t = c.get_chain_type()
        self.assertEqual(t,IMP.atom.Polydeoxyribonucleotide)

        c.set_chain_type(IMP.atom.RNA)
        t = c.get_chain_type()
        self.assertEqual(t,IMP.atom.Polyribonucleotide)

if __name__ == '__main__':
    IMP.test.main()
