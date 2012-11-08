import IMP
import IMP.test
import IMP.cnmultifit
import os
from IMP.cnmultifit import param

class Tests(IMP.test.TestCase):

    def test_param_help(self):
        """Test param module help"""
        self.check_runnable_python_module("IMP.cnmultifit.param")

    def test_param_usage(self):
        """Test param module incorrect usage"""
        r = self.run_python_module("IMP.cnmultifit.param", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_param_run(self):
        """Test run of param module"""
        self.run_python_module(param,
                               ['-o', 'test.output', '-i', 'test.int',
                                '--params', 'test.params',
                                '--model', 'test.model',
                                '--numsols', '42', '--',
                                '7', 'testmonomer.pdb', 'test.mrc', '8.0',
                                '4.0', '5.0', '-10.0', '-20.0', '-30.0'])
        contents = open('test.params').read()
        self.assertIn('output test.output', contents)
        self.assertIn('intermediate test.int', contents)
        self.assertIn('model test.model', contents)
        self.assertIn('fitting 42', contents)
        self.assertIn('cn_symm_deg 7', contents)
        self.assertIn('monomer testmonomer.pdb', contents)
        self.assertIn('density test.mrc', contents)
        self.assertIn('density_resolution 8.0', contents)
        self.assertIn('density_spacing 4.0', contents)
        self.assertIn('density_threshold 5.0', contents)
        self.assertIn('density_origin -10.0 -20.0 -30.0', contents)
        os.unlink('test.params')

if __name__ == '__main__':
    IMP.test.main()
