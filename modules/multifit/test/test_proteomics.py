import IMP
import os
import IMP.test
import IMP.multifit
from IMP.multifit import proteomics


class Tests(IMP.test.TestCase):

    def test_proteomics_help(self):
        """Test proteomics module help"""
        self.check_runnable_python_module("IMP.multifit.proteomics")

    def test_proteomics_usage(self):
        """Test proteomics module incorrect usage"""
        r = self.run_python_module("IMP.multifit.proteomics", [])
        out, err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_proteomics_run(self):
        """Test proteomics module run"""
        self.run_python_module(proteomics,
                               [self.get_input_file_name(
                                   'autoprot.asmb.input'),
                                self.get_input_file_name(
                                    'autoprot.anchors.txt'),
                                'autoprot.out'])
        # Note that anchors 1 and 2 both map to apC, so there is a
        # duplicate edge (0-1 and 2-0). Make sure that there is only
        # one apA|apC pair output:
        contents = open("autoprot.out").readlines()
        contents = [x.rstrip('\r\n') for x in contents]
        self.assertEqual(contents,
                         ["|proteins|",
                          "|apA|1|2|nn|nn|",
                          "|apB|1|2|nn|nn|",
                          "|apC|1|1|nn|nn|",
                          "|interactions|",
                          "|residue-xlink|",
                          "|ev-pairs|",
                          "|apA|apC|",
                          "|apA|apB|"])
        # Makes sure the file is in acceptable proteomics format
        d = IMP.multifit.read_proteomics_data("autoprot.out")
        d.set_was_used(True)
        os.unlink("autoprot.out")

if __name__ == '__main__':
    IMP.test.main()
