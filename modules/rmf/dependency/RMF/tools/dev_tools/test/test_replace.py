import unittest
import subprocess
import os
import utils

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
replace_py = os.path.join(TOPDIR, 'replace.py')

class Tests(unittest.TestCase):

    def test_replace(self):
        """Test replace.py script"""
        with utils.TempDir() as tmpdir:
            filenames = [os.path.join(tmpdir, x) for x in ['a', 'b', 'c']]
            os.mkdir(filenames[0])
            utils.write_file(filenames[1], "foo bar baz")
            utils.write_file(filenames[2], "bar baz")
            p = subprocess.Popen([replace_py, 'foo', 'bar'] + filenames,
                                 cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)
            self.assertEqual(utils.read_file(filenames[1]), "bar bar baz")
            self.assertEqual(utils.read_file(filenames[2]), "bar baz")

    def test_replace_usage(self):
        """Test replace.py script usage"""
        p = subprocess.Popen([replace_py])
        stdout, stderr = p.communicate()
        self.assertEqual(p.returncode, 1)

if __name__ == '__main__':
    unittest.main()
