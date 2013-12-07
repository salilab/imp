import IMP
import os
import IMP.test
import IMP.multifit
from IMP.cnmultifit import surface


class Tests(IMP.test.TestCase):

    def test_surface_help(self):
        """Test surface module help"""
        self.check_runnable_python_module("IMP.cnmultifit.surface")

    def test_surface_usage(self):
        """Test surface module incorrect usage"""
        r = self.run_python_module("IMP.cnmultifit.surface", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_surface_run(self):
        """Test surface module run"""
        def assert_surface_ok(leaves, outfile, density, rp):
            self.assertEqual(len(leaves), 2)
            self.assertEqual(outfile, 'test.pdb.ms')
            self.assertEqual(density, 5.0)
            self.assertEqual(rp, 3.0)
        open('test.pdb', 'w').write("""
ATOM      1  N   ALA A   2     -27.425   4.153 -19.130  1.00  0.00           N
ATOM      2  CA  ALA A   2     -18.390   4.442 -18.049  1.00  0.00           C
""")
        old = IMP.multifit.write_connolly_surface
        try:
            IMP.multifit.write_connolly_surface = assert_surface_ok
            self.run_python_module(surface, ['--radius', '3.0',
                                             '--density', '5.0', 'test.pdb'])
        finally:
            IMP.multifit.write_connolly_surface = old
        os.unlink('test.pdb')

if __name__ == '__main__':
    IMP.test.main()
