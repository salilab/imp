import IMP
import os
import shutil
import IMP.test
import IMP.multifit
from IMP.multifit import surface

class Tests(IMP.test.TestCase):

    def test_surface_help(self):
        """Test surface module help"""
        self.check_runnable_python_module("IMP.multifit.surface")

    def test_surface_usage(self):
        """Test surface module incorrect usage"""
        r = self.run_python_module("IMP.multifit.surface", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_surface_run(self):
        """Test surface module run"""
        self.surface_calls = 0
        expected_outfile = ['testA.ms', 'testB.ms']
        def assert_surface_ok(leaves, outfile, density, rp):
            self.assertEqual(len(leaves), 2)
            self.assertEqual(outfile, expected_outfile[self.surface_calls])
            self.assertEqual(density, 10.0)
            self.assertAlmostEqual(rp, 1.8, delta=1e-5)
            self.surface_calls += 1

        open('test1.pdb', 'w').write("""
ATOM      1  N   ALA A   2     -27.425   4.153 -19.130  1.00  0.00           N
ATOM      2  CA  ALA A   2     -18.390   4.442 -18.049  1.00  0.00           C
""")
        shutil.copy('test1.pdb', 'test2.pdb')
        open('test.asmb', 'w').write(
"""subunit header
test1|test1.pdb|testA.ms||1||1|||
test2|test2.pdb|testB.ms||1||1|||
density header
|15.0|2|300|30.0|4.0|-96.0|||||
""")
        old = IMP.multifit.write_connolly_surface
        try:
            IMP.multifit.write_connolly_surface = assert_surface_ok
            self.run_python_module(surface, ['test.asmb'])
        finally:
            IMP.multifit.write_connolly_surface = old
        self.assertEqual(self.surface_calls, 2)
        os.unlink('test.asmb')
        os.unlink('test1.pdb')
        os.unlink('test2.pdb')

if __name__ == '__main__':
    IMP.test.main()
