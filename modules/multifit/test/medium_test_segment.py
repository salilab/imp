import IMP
import os
import IMP.test
import IMP.multifit
from IMP.multifit import segment


class Tests(IMP.test.TestCase):

    def test_segment_help(self):
        """Test segment module help"""
        self.check_runnable_python_module("IMP.multifit.segment")

    def test_segment_usage(self):
        """Test segment module incorrect usage"""
        r = self.run_python_module("IMP.multifit.segment", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_segment_run(self):
        """Test segment module run"""
        self.run_python_module(segment,
                               [self.get_input_file_name(
                                   'twoblobs-4.0.mrc'), '2', '0.1',
                                'segment_out.pdb', '--seg', 'segment_out'])
        for i in range(2):
            os.unlink('segment_out_%d.mrc' % i)
        os.unlink('segment_out.pdb')
        os.unlink('load_configuration.cmd')

if __name__ == '__main__':
    IMP.test.main()
