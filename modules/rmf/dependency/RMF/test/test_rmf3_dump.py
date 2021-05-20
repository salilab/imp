import unittest
import RMF
import subprocess


class Tests(unittest.TestCase):

    def test_help(self):
        """Test rmf3_dump --help"""
        p = subprocess.Popen(['rmf3_dump', '--help'], stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(out, "")
        self.assertIn("Dump frames from an rmf3 file", err)
        self.assertEqual(p.returncode, 1)

    def test_version(self):
        """Test rmf3_dump --version"""
        p = subprocess.Popen(['rmf3_dump', '--version'], stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        out, err = p.communicate()
        self.assertEqual(err, "")
        self.assertIn("RMF version", out)
        self.assertEqual(p.returncode, 0)


if __name__ == '__main__':
    unittest.main()
