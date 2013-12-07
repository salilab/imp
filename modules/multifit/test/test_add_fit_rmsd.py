import IMP
import os
import IMP.test
import IMP.multifit
from IMP.multifit import add_fit_rmsd


class Tests(IMP.test.TestCase):

    def test_add_fit_rmsd_help(self):
        """Test add_fit_rmsd module help"""
        self.check_runnable_python_module("IMP.multifit.add_fit_rmsd")

    def test_add_fit_rmsd_usage(self):
        """Test add_fit_rmsd module incorrect usage"""
        r = self.run_python_module("IMP.multifit.add_fit_rmsd", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_add_fit_rmsd_run(self):
        """Test add_fit_rmsd module run"""
        self.run_python_module(add_fit_rmsd, [
            self.get_input_file_name('fit_rmsd.input'),
            self.get_input_file_name('fit_rmsd.prot'),
            self.get_input_file_name('fit_rmsd.mapping'),
            self.get_input_file_name('fit_rmsd.align.param')])

        out_fn = self.get_input_file_name('fit_rmsdA_fitting.txt.RMSD')
        recs = IMP.multifit.read_fitting_solutions(out_fn)
        self.assertEqual(len(recs), 1)
        # Simple translation, so RMSD should be the same for all-atom
        # or CA-only
        self.assertAlmostEqual(recs[0].get_rmsd_to_reference(),
                               2.45, delta=0.01)
        os.unlink(out_fn)

if __name__ == '__main__':
    IMP.test.main()
