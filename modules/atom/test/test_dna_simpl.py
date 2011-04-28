import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Test simplifying DNA"""
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("dna.pdb"), m)

        hs= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(h.get_child(0)), 1)
        #w= IMP.display.PymolWriter("1.pym")
        #g= IMP.display.HierarchyGeometry(hs)
        #w.add_geometry(g)
        for h in hs.get_children():
            d=IMP.core.XYZR(h)
            r= d.get_radius()
            self.assert_(r>4 and r < 5)

if __name__ == '__main__':
    IMP.test.main()
