import IMP
import os
import sys
import IMP.test
import IMP.multifit
from IMP.multifit import refine_fft

class Tests(IMP.test.TestCase):

    def test_refine_fft_help(self):
        """Test refine_fft module help"""
        self.check_runnable_python_module("IMP.multifit.refine_fft")

    def test_refine_fft_usage(self):
        """Test refine_fft module incorrect usage"""
        r = self.run_python_module("IMP.multifit.refine_fft", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_refine_fft_run(self):
        """Test refine_fft module run"""
        self.run_python_module(refine_fft,
                 [self.get_input_file_name('refine_fft.asmb.input'),
                  self.get_input_file_name('refine_fft.asmb.input.refined'),
                  self.get_input_file_name('refine_fft.proteomics'),
                  self.get_input_file_name('refine_fft.indexes'),
                  self.get_input_file_name('refine_fft.combinations'), '0'])
        os.unlink(self.get_input_file_name('refine_fftA.fitting.refined.out'))
        os.unlink(self.get_input_file_name('refine_fftB.fitting.refined.out'))

if __name__ == '__main__':
    IMP.test.main()
