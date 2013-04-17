import IMP
import os
import IMP.test
import IMP.atom
import IMP.multifit
from IMP.multifit import anchors

class Tests(IMP.test.TestCase):

    def test_anchors_help(self):
        """Test anchors module help"""
        self.check_runnable_python_module("IMP.multifit.anchors")

    def test_anchors_usage(self):
        """Test anchors module incorrect usage"""
        r = self.run_python_module("IMP.multifit.anchors", [])
        out,err = r.communicate()
        self.assertEqual(out, "")
        self.assertIn("incorrect number of arguments", err)
        self.assertNotEqual(r.returncode, 0)

    def test_anchors_run(self):
        """Test anchors module run"""
        open("test.asmb", "w").write("""subunit header
A|%s|||1||1|||
B|%s|||1||1|||
density header
%s|4.0|1.33|0.02|1.1|11.8|-6.9|||||
""" % (self.get_input_file_name("twoblobsA.pdb"),
       self.get_input_file_name("twoblobsB.pdb"),
       self.get_input_file_name("twoblobs-4.0.mrc")))
        self.run_python_module(anchors, ['test.asmb', 'test.out'])

        # Check PDB and cmm output
        self.assertEqual(len(open('test.out.pdb').readlines()), 2)
        self.assertEqual(len(open('test.out.cmm').readlines()), 5)

        # Check text output
        txt = open('test.out.txt').readlines()
        self.assertEqual(len(txt), 5)
        self.assertEqual(txt[0].rstrip('\r\n'), '|points|')
        self.assertEqual(txt[3].rstrip('\r\n'), '|edges|')
        self.assertEqual(txt[4].rstrip('\r\n'), '|0|1|')
        pts = []
        pts.append([float(x) for x in txt[1].split('|')[2:5]])
        pts.append([float(x) for x in txt[2].split('|')[2:5]])
        pts.sort()
        self.assertAlmostEqual(pts[0][0], 24.0, delta=1.0)
        self.assertAlmostEqual(pts[0][1], 11.6, delta=1.0)
        self.assertAlmostEqual(pts[0][2],  8.2, delta=1.0)
        self.assertAlmostEqual(pts[1][0], 26.7, delta=1.0)
        self.assertAlmostEqual(pts[1][1], 14.6, delta=1.0)
        self.assertAlmostEqual(pts[1][2],  5.1, delta=1.0)

        os.unlink('test.asmb')
        os.unlink('test.out.pdb')
        os.unlink('test.out.txt')
        os.unlink('test.out.cmm')

if __name__ == '__main__':
    IMP.test.main()
