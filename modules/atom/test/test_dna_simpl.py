from __future__ import print_function
import os
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display


class Tests(IMP.test.TestCase):

    def test_bonded(self):
        """Test simplifying DNA"""
        m = IMP.Model()
        h = IMP.atom.read_pdb(self.get_input_file_name("dna.pdb"), m)

        hs = IMP.atom.create_simplified_along_backbone(
            IMP.atom.Chain(h.get_child(0)), 1)
        w = IMP.display.PymolWriter("1.pym")
        g = IMP.atom.HierarchyGeometry(hs)
        w.add_geometry(g)
        for h in hs.get_children():
            d = IMP.core.XYZR(h)
            r = d.get_radius()
            print(r)
            self.assertTrue(r > 3.5 and r < 5.5)
        del w
        os.unlink('1.pym')

    def test_bonded2(self):
        """Test simplifying DNA 2"""
        m = IMP.Model()
        h = IMP.atom.read_pdb(self.get_input_file_name("dna.pdb"), m)

        hs = IMP.atom.create_simplified_along_backbone(
            IMP.atom.Chain(h.get_child(0)), 2)
        w = IMP.display.PymolWriter("2.pym")
        g = IMP.atom.HierarchyGeometry(hs)
        w.add_geometry(g)
        for h in hs.get_children():
            d = IMP.core.XYZR(h)
            r = d.get_radius()
            print(r)
            self.assertTrue(r > 4 and r < 6.8)
        del w
        os.unlink('2.pym')


if __name__ == '__main__':
    IMP.test.main()
