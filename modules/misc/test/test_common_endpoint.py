import IMP
import IMP.test
import IMP.misc
import IMP.atom
import math

class Tests(IMP.test.TestCase):
    """Tests for tunnel scores"""

    def test_score(self):
        """Test common endpoints container"""
        m= IMP.Model()
        lbp=None
        bonds=[]
        ps=[]
        for i in range(0,4):
            p= IMP.Particle(m)
            bp= IMP.atom.Bonded.setup_particle(p)
            if lbp is not None:
                bonds.append(IMP.atom.create_custom_bond(lbp, bp, 1,1))
            lbp= bp
            ps.append(p)
        cepc= IMP.misc.CommonEndpointPairFilter()
        self.assertTrue(cepc.get_value((bonds[0].get_particle(),
                                                              bonds[1].get_particle())))
        self.assertTrue(cepc.get_value((bonds[2].get_particle(),
                                                              bonds[1].get_particle())))
        self.assertFalse(cepc.get_value((bonds[0].get_particle(),
                                                                  bonds[2].get_particle())))

if __name__ == '__main__':
    IMP.test.main()
