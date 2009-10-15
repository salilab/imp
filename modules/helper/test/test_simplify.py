import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class SimplifyTests(IMP.test.TestCase):
    def test_simplify_2(self):
        """Test protein simplification 2"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p= IMP.atom.read_pdb(self.get_input_file_name('single_protein.pdb'), m)
        #IMP.atom.show_molecular_hierarchy(p)
        IMP.atom.add_radii(p)
        s= IMP.helper.create_simplified(p, 20)
        ls= IMP.core.get_leaves(s)
        for q in []:
            d= IMP.core.XYZR(q.get_particle())
            print ".sphere " +str(d.get_coordinates()[0]) + " "\
                + str(d.get_coordinates()[1]) + " "\
                + str(d.get_coordinates()[2]) + " "\
                + str(d.get_radius())
        print "level is " +str(IMP.get_log_level())


    def test_simplify_by_residue(self):
        """Test protein simplification by residues"""
        IMP.set_log_level(IMP.SILENT)#VERBOSE)
        m= IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name('single_protein.pdb'), m)
        num_residues=len(IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE))
        IMP.atom.add_radii(mh)
        residual_cond=lambda x: x if x==0 else 1
        for res_segment in [5,10,20,30,num_residues]:
            mh_simp= IMP.helper.create_simplified_by_residue(mh, res_segment)
            self.assertEqual(num_residues/res_segment+1*residual_cond(num_residues%res_segment), len(IMP.core.get_leaves(mh_simp)))
if __name__ == '__main__':
    unittest.main()
