import IMP
import IMP.test
import IMP.atom
import IMP.cnmultifit
import os.path
from IMP.cnmultifit import chimera_models

class Tests(IMP.test.TestCase):

    def test_chimera_models_help(self):
        """Test chimera_models module help"""
        self.check_runnable_python_module("IMP.cnmultifit.chimera_models")

    def test_chimera_models_usage(self):
        """Test chimera_models module incorrect usage"""
        r = self.run_python_module("IMP.cnmultifit.chimera_models", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_chimera_models_run(self):
        """Test chimera_models module run"""
        open('test.pdb', 'w').write("""
ATOM      1  N   ALA A   2     -27.425   4.153 -19.130  1.00  0.00           N
ATOM      2  CA  ALA A   2     -18.390   4.442 -18.049  1.00  0.00           C
""")
        self.run_python_module(chimera_models,
              ['test.pdb', '7',
               self.get_input_file_name('multifit.chimera.output'), '2',
               'test.output'])
        m = IMP.kernel.Model()
        for expected in ['test.output.000.pdb', 'test.output.001.pdb']:
            h = IMP.atom.read_pdb(expected, m)
            atoms = IMP.atom.get_by_type(h, IMP.atom.ATOM_TYPE)
            self.assertEqual(len(atoms), 14)
            os.unlink(expected)
        # Only 2 models were requested, even though the transformations file
        # contains information for more
        self.assertFalse(os.path.exists('test.output.002.pdb'))
        os.unlink('test.pdb')

if __name__ == '__main__':
    IMP.test.main()
