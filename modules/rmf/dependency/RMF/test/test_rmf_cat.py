import unittest
import os
import RMF
import subprocess


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

    def test_cat_mismatch(self):
        """Test rmf_cat of mismatched files"""
        p = subprocess.Popen(
            ['rmf_cat', RMF._get_test_input_file_path("simple.rmf"),
             RMF._get_test_input_file_path("rep_and_geom.rmf"),
             'test_cat_mismatch.rmf'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        self.assertIn("have different structure", err)
        self.assertEqual(p.returncode, 1)

    def test_cat_ok(self):
        """Test rmf_cat of similar files"""
        p = subprocess.Popen(
            ['rmf_cat', RMF._get_test_input_file_path("simple.rmf"),
             RMF._get_test_input_file_path("simple-new.rmf"),
             'test_cat_ok.rmf'],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(out, "")
        self.assertEqual(err, "")
        self.assertEqual(p.returncode, 0)
        os.unlink('test_cat_ok.rmf')


if __name__ == '__main__':
    unittest.main()
