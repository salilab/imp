import IMP
import IMP.test
import IMP.cnmultifit
from IMP.cnmultifit import build


class Tests(IMP.test.TestCase):

    def test_build_help(self):
        """Test build module help"""
        self.check_runnable_python_module("IMP.cnmultifit.build")

    def test_build_usage(self):
        """Test build module incorrect usage"""
        r = self.run_python_module("IMP.cnmultifit.build", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_build_run(self):
        """Test build module run"""
        def assert_fitting_ok(param, chimera):
            self.assertEqual(param, "params")
            self.assertEqual(chimera, "chimera.out")
        old = IMP.cnmultifit.do_all_fitting
        try:
            IMP.cnmultifit.do_all_fitting = assert_fitting_ok
            self.run_python_module(build, ['--chimera', 'chimera.out',
                                           'params'])
        finally:
            IMP.cnmultifit.do_all_fitting = old

if __name__ == '__main__':
    IMP.test.main()
