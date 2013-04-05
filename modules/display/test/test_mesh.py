import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display
import os

class Tests(IMP.test.TestCase):
    def test_3(self):
        """Testing skin surface"""
        if not IMP.display.IMP_DISPLAY_HAS_IMP_CGAL:
            self.skipTest("IMP.cgal is disabled")
        m= IMP.Model()
        print "reading"
        h= IMP.atom.read_pdb(self.get_input_file_name("1d3d-protein.pdb"), m)
        hs = IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(IMP.atom.get_by_type(h, IMP.atom.CHAIN_TYPE)[0]), 10)
        ss=[]
        print "stripping", len(IMP.atom.get_leaves(hs))
        for l in IMP.atom.get_leaves(hs):
            ss.append(IMP.core.XYZR(l).get_sphere())
        print "skinning"
        g= IMP.display.SkinSurfaceGeometry(ss)
        g.set_name("surface")
        print "writing"
        w= IMP.display.PymolWriter(self.get_tmp_file_name("skin.pym"))
        w.add_geometry(g)

if __name__ == '__main__':
    IMP.test.main()
