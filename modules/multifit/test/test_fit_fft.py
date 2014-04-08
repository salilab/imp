import IMP
import os
import sys
import IMP.test
import IMP.multifit
from IMP.multifit import fit_fft


class Tests(IMP.test.TestCase):

    def test_fit_fft_help(self):
        """Test fit_fft module help"""
        self.check_runnable_python_module("IMP.multifit.fit_fft")

    def test_fit_fft_usage(self):
        """Test fit_fft module incorrect usage"""
        r = self.run_python_module("IMP.multifit.fit_fft", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_fit_fft_run(self):
        """Test fit_fft module run"""
        self.run_python_module(fit_fft,
                               [self.get_input_file_name('twoblobs.asmb.input')])
        os.unlink(self.get_input_file_name('twoblobsA.fitting.out'))
        os.unlink(self.get_input_file_name('twoblobsB.fitting.out'))

    def test_fit_fft_run_parallel(self):
        """Test fit_fft module run in parallel"""
        try:
            import multiprocessing
        except ImportError:
            self.skipTest("multiprocessing module not available")
        if sys.platform == 'win32' and 'WINELOADERNOEXEC' in os.environ:
            self.skipTest("multiprocessing module does not work with Wine")
        self.run_python_module(fit_fft,
                               ['-c', '2', self.get_input_file_name('twoblobs.asmb.input')])
        os.unlink(self.get_input_file_name('twoblobsA.fitting.out'))
        os.unlink(self.get_input_file_name('twoblobsB.fitting.out'))

if __name__ == '__main__':
    IMP.test.main()
