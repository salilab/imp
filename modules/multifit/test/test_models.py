import IMP
import os
import IMP.test
import IMP.multifit
from IMP.multifit import models


class Tests(IMP.test.TestCase):

    def test_models_help(self):
        """Test models module help"""
        self.check_runnable_python_module("IMP.multifit.models")

    def test_models_usage(self):
        """Test models module incorrect usage"""
        r = self.run_python_module("IMP.multifit.models", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_models_run(self):
        """Test models module run"""
        self.run_python_module(models,
                               [self.get_input_file_name('models.asmb.input'),
                                self.get_input_file_name(
                                    'models.proteomics.input'),
                                self.get_input_file_name(
                                    'models.indexes.input'),
                                self.get_input_file_name(
                                    'models.combinations'),
                                'models.asmb'])
        for i in range(3):
            os.unlink('models.asmb.%d.pdb' % i)
        self.run_python_module(models,
                               ['-m', '2', self.get_input_file_name('models.asmb.input'),
                                self.get_input_file_name(
                                    'models.proteomics.input'),
                                self.get_input_file_name(
                                    'models.indexes.input'),
                                self.get_input_file_name(
                                    'models.combinations'),
                                'models.asmb'])
        for i in range(2):
            os.unlink('models.asmb.%d.pdb' % i)
        self.assertFalse(os.path.exists('models.asmb.2.pdb'))

if __name__ == '__main__':
    IMP.test.main()
