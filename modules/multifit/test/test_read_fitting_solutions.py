import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.restrainer
import os
import time

class Tests(IMP.test.TestCase):
    """Tests for reading fitting solutions"""
    def setup_filenames(self):
        self.protein_fns=[
            self.get_input_file_name("1z5s_A.pdb"),
            self.get_input_file_name("1z5s_B.pdb"),
            self.get_input_file_name("1z5s_C.pdb"),
            self.get_input_file_name("1z5s_D.pdb")]
        self.fitting_solutions_fns=[
            self.get_input_file_name("1z5s_A_fitting_solutions.txt")]
            # self.get_input_file_name("1z5s_B_fitting_solutions.txt"),
            # self.get_input_file_name("1z5s_C_fitting_solutions.txt"),
            # self.get_input_file_name("1z5s_D_fitting_solutions.txt")]
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        self.setup_filenames()
        self.mdl=IMP.Model()
        self.mhs=IMP.atom.Hierarchies()
        self.ca_sel=IMP.atom.CAlphaPDBSelector()
        #load the components
        for prot_fn in self.protein_fns:
            mh=IMP.atom.read_pdb(prot_fn,self.mdl,self.ca_sel)
            self.mhs.append(mh)
            IMP.atom.add_radii(mh)
            IMP.atom.setup_as_rigid_body(mh)
    def test_reading_fitting_solutions(self):
        """Check correct parsing of fitting solutions file"""
        sols = IMP.multifit.read_fitting_solutions(self.fitting_solutions_fns[0])
        self.assertEqual(len(sols),13)
        self.assertEqual(sols[0].get_match_size(),9)
        self.assertAlmostEqual(sols[1].get_match_average_distance(),3.2,
                               delta=0.01)
        self.assertAlmostEqual(sols[2].get_rmsd_to_reference(),2.98,
                               delta=0.01)
        self.assertAlmostEqual(sols[2].get_envelope_penetration_score(),4.5,
                               delta=0.01)
    def test_writing_and_reading_fitting_solutions(self):
        """Check the fitting solutions writer"""
        sols = IMP.multifit.read_fitting_solutions(self.fitting_solutions_fns[0])
        IMP.multifit.write_fitting_solutions("temp.txt",sols)
        sols = IMP.multifit.read_fitting_solutions("temp.txt")
        self.assertEqual(len(sols),13)
        self.assertEqual(sols[0].get_match_size(),9)
        self.assertAlmostEqual(sols[1].get_match_average_distance(),3.2,
                               delta=0.01)
        self.assertAlmostEqual(sols[2].get_rmsd_to_reference(),2.98,
                               delta=0.01)
        os.unlink("temp.txt")
if __name__ == '__main__':
    IMP.test.main()
