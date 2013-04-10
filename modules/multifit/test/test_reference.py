import IMP
import IMP.test
import IMP.multifit
from IMP.multifit import reference

class Tests(IMP.test.TestCase):

    def test_reference_help(self):
        """Test reference module help"""
        self.check_runnable_python_module("IMP.multifit.reference")

    def test_reference_usage(self):
        """Test reference module incorrect usage"""
        r = self.run_python_module("IMP.multifit.reference", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_reference_run(self):
        """Test reference module run"""
        r = self.run_python_module(reference,
                    [self.get_input_file_name('reference.asmb.input'),
                     self.get_input_file_name('reference.prot.input'),
                     self.get_input_file_name('reference.indexes.input'),
                     self.get_input_file_name('reference.comb.input')])
        self.assertEqual(len(r), 3)
        self.assertEqual(len(r[1]), 2)
        rmsd = r[1][0]
        self.assertAlmostEqual(rmsd, 45.5, delta=0.1)
        dist, ang = r[1][1][0]
        self.assertAlmostEqual(dist, 1.47, delta=0.01)
        self.assertAlmostEqual(ang, 0.0, delta=0.01)
        dist, ang = r[1][1][1]
        self.assertAlmostEqual(dist, 53.32, delta=0.01)
        self.assertAlmostEqual(ang, 1.82, delta=0.01)

if __name__ == '__main__':
    IMP.test.main()
