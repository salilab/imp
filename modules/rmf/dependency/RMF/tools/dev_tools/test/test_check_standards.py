import unittest
import os
import sys
import re
import utils

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.insert(0, TOPDIR)
try:
    import check_standards
finally:
    del sys.path[0]

class Tests(unittest.TestCase):

    def test_do_not_commit(self):
        """Test _check_do_not_commit function"""
        errors = []
        check_standards._check_do_not_commit("", 'foo', 0, errors)
        self.assertEqual(len(errors), 0)
        check_standards._check_do_not_commit("DO NOT COMMIT", 'foo', 0, errors)
        self.assertEqual(errors,
                   ['foo:1: Line contains the string "DO NOT COMMIT"'])

    def test_get_file(self):
        """Test get_file function"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, 'foo')
            utils.write_file(fname, 'foobar')
            fh, fn = check_standards.get_file(fname)
            self.assertEqual(fn, fname)
            self.assertEqual(fh.read(), 'foobar')

    def test_file_matches_re(self):
        """Test file_matches_re function"""
        excludes = [re.compile('foo')]
        self.assertFalse(check_standards.file_matches_re("bar", excludes))
        self.assertFalse(check_standards.file_matches_re("bar", []))
        self.assertTrue(check_standards.file_matches_re("foobar", excludes))

    def test_get_all_files(self):
        """Test get_all_files function"""
        with utils.RunInTempDir() as tmpdir:
            for subdir in ('.sconf_temp', os.path.join('build', 'include'),
                           'ok'):
                os.makedirs(subdir)
                utils.write_file(os.path.join(subdir, "foo"), "")
            for f in ('.foo', 'foo', 'bar'):
                utils.write_file(f, "")
            fs = sorted(check_standards.get_all_files())
            self.assertEqual(fs, ['./bar', './foo', './ok/foo'])

if __name__ == '__main__':
    unittest.main()
