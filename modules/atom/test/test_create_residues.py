import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Check creating residues from CHARMM topology """
        m= IMP.Model()
        sequence= "FSFGAKSKENKAGATSKPAFSFG" #"GLFGQNNQQQGSGLFG"
        seg= IMP.atom.CHARMMSegmentTopology()
        ff= IMP.atom.get_default_CHARMM_parameters()
        for r in sequence:
            rt= IMP.atom.get_residue_type(r)
            seg.add_residue(IMP.atom.CHARMMResidueTopology(ff.get_residue_topology(rt)))
        top= IMP.atom.CHARMMTopology()
        top.add_segment(seg)
        top.apply_default_patches()
        h= top.create_hierarchy(m)
        atoms= IMP.atom.get_by_type(h, IMP.atom.ATOM_TYPE)
        print atoms
        IMP.atom.show_molecular_hierarchy(h)
        self.assert_(len(atoms) > 0)

if __name__ == '__main__':
    unittest.main()
