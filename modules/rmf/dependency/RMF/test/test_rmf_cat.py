import unittest
import os
import RMF
import subprocess


def make_input_rmf(fname, add_node=True, static_value=1):
    fh = RMF.create_rmf_file(fname)
    fh.add_frame("root", RMF.FRAME)
    rn = fh.get_root_node()
    ch = rn.add_child("ch1", RMF.REPRESENTATION)
    if add_node:
        ch = rn.add_child("ch2", RMF.REPRESENTATION)
        cat = fh.get_category("MyCat")
        key = fh.get_key(cat, "MyKey", RMF.int_tag)
        fh.get_root_node().set_static_value(key, static_value)


class Tests(unittest.TestCase):

    def test_help(self):
        """Test rmf_cat --help"""
        p = subprocess.Popen(['rmf_cat', '--help'], stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(out, "")
        self.assertIn("Combine two or more rmf files", err)
        self.assertEqual(p.returncode, 1)

    def test_version(self):
        """Test rmf_cat --version"""
        p = subprocess.Popen(['rmf_cat', '--version'], stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(err, "")
        self.assertIn("RMF version", out)
        self.assertEqual(p.returncode, 0)

    def test_cat_mismatch_structure(self):
        """Test rmf_cat of structure-mismatched files"""
        make_input_rmf("mismatch_struc_1.rmf")
        make_input_rmf("mismatch_struc_2.rmf", add_node=False)
        p = subprocess.Popen(
            ['rmf_cat', "mismatch_struc_1.rmf", "mismatch_struc_2.rmf",
             'mismatch_struc.rmf'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        self.assertIn("have different structure", err)
        self.assertEqual(p.returncode, 1)
        os.unlink('mismatch_struc_1.rmf')
        os.unlink('mismatch_struc_2.rmf')

    def test_cat_mismatch_static(self):
        """Test rmf_cat of static-frame-mismatched files"""
        make_input_rmf("mismatch_static_1.rmf", static_value=1)
        make_input_rmf("mismatch_static_2.rmf", static_value=2)
        p = subprocess.Popen(
            ['rmf_cat', "mismatch_static_1.rmf", "mismatch_static_2.rmf",
             'mismatch_static.rmf'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        self.assertIn("have different static frames", err)
        self.assertEqual(p.returncode, 1)
        os.unlink('mismatch_static_1.rmf')
        os.unlink('mismatch_static_2.rmf')

    def test_cat_mismatch_force(self):
        """Test forced rmf_cat of mismatched files"""
        make_input_rmf("mismatch_force_1.rmf", static_value=1)
        make_input_rmf("mismatch_force_2.rmf", static_value=2)
        p = subprocess.Popen(
            ['rmf_cat', "mismatch_force_1.rmf", "mismatch_force_2.rmf",
             '--force', 'mismatch_force.rmf'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(out, "")
        self.assertEqual(err, "")
        self.assertEqual(p.returncode, 0)
        os.unlink('mismatch_force_1.rmf')
        os.unlink('mismatch_force_2.rmf')
        os.unlink('mismatch_force.rmf')

    def test_cat_ok(self):
        """Test rmf_cat of similar files"""
        make_input_rmf("cat_ok_1.rmf")
        make_input_rmf("cat_ok_2.rmf")
        p = subprocess.Popen(
            ['rmf_cat', "cat_ok_1.rmf", "cat_ok_2.rmf", 'cat_ok.rmf'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(out, "")
        self.assertEqual(err, "")
        self.assertEqual(p.returncode, 0)
        os.unlink('cat_ok_1.rmf')
        os.unlink('cat_ok_2.rmf')
        os.unlink('cat_ok.rmf')


if __name__ == '__main__':
    unittest.main()
