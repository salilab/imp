import IMP
import os
import IMP.test
import IMP.multifit
from IMP.multifit import score


class Tests(IMP.test.TestCase):

    def test_score_help(self):
        """Test score module help"""
        self.check_runnable_python_module("IMP.multifit.score")

    def test_score_usage(self):
        """Test score module incorrect usage"""
        r = self.run_python_module("IMP.multifit.score", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_score_run(self):
        """Test score module run"""
        self.run_python_module(score,
                               [self.get_input_file_name('score.asmb.input'),
                                self.get_input_file_name(
                                    'score.proteomics.input'),
                                self.get_input_file_name(
                                    'score.indexes.input'),
                                self.get_input_file_name('score.align.param'),
                                self.get_input_file_name('score.combinations'),
                                'scores.asmb'])
        for i in range(3):
            os.unlink('model.%d.pdb' % i)
        self.assertFalse(os.path.exists('model.3.pdb'))
        os.unlink('scores.asmb')

if __name__ == '__main__':
    IMP.test.main()
