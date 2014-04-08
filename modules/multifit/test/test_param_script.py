import IMP
import IMP.test
import IMP.multifit
import os
from IMP.multifit import param


class Tests(IMP.test.TestCase):

    def test_param_help(self):
        """Test param module help"""
        self.check_runnable_python_module("IMP.multifit.param")

    def test_param_usage(self):
        """Test param module incorrect usage"""
        r = self.run_python_module("IMP.multifit.param", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_param_run(self):
        """Test run of param module"""
        f = open('testasmb.subunits', 'w')
        f.write("testA %s 1\n" % self.get_input_file_name("1z5s_A.pdb"))
        f.write("testB %s 0\n" % self.get_input_file_name("1z5s_B.pdb"))
        f.close()
        self.run_python_module(param,
                               ['-i', 'testasmb.input',
                                '--model', 'test.model',
                                '--anchor_dir', 'ancdir',
                                '--fit_dir', 'fitdir', '--',
                                'testasmb', 'testasmb.subunits',
                                '10', 'testasmb.mrc', '15', '2', '300',
                                '30', '4', '-96'])
        lines = open('testasmb.input').readlines()
        self.assertEqual(len(lines), 5)
        spl = lines[1].rstrip('\r\n').split('|')
        self.assertEqual(len(spl), 10)
        self.assertEqual(spl[0], 'testA')
        self.assertTrue(spl[1].endswith('1z5s_A.pdb'))
        self.assertTrue(spl[2].endswith('1z5s_A.pdb.ms'))
        self.assertEqual(spl[3], 'ancdir/testA_anchors.txt')
        self.assertEqual(spl[4], '15')
        self.assertEqual(spl[5], 'ancdir/testA_fine_anchors.txt')
        self.assertEqual(spl[6], '156')
        self.assertEqual(spl[7], 'fitdir/testA_fitting.txt')
        self.assertEqual(spl[8], '')
        self.assertEqual(spl[9], '')

        self.assertEqual(lines[4].rstrip('\r\n'),
                         "testasmb.mrc|15.0|2|300|30.0|4.0|-96.0|"
                         "ancdir/testasmb_em_coarse_anchors.txt|"
                         "ancdir/testasmb_em_coarse_anchors_FINE.txt|"
                         "ancdir/testasmb_em_fine_anchors.txt|"
                         "ancdir/testasmb_em_fine_anchors_FINE.txt|")
        for f in ('testasmb.alignment.param',
                  'testasmb.input', 'testasmb.subunits'):
            os.unlink(f)

if __name__ == '__main__':
    IMP.test.main()
