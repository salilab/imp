import unittest
import subprocess
import os
import utils
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(TOPDIR)
import python_tools

class Tests(unittest.TestCase):

    def test_to_from_cmake_path(self):
        """Test to/from_cmake_path functions"""
        # todo: check both paths (by reimporting the module)
        if os.sep == '\\': # Windows
            self.assertEqual(python_tools.to_cmake_path('\\foo\\bar'),
                             '/foo/bar')
            self.assertEqual(python_tools.from_cmake_path('/foo/bar'),
                             '\\foo\\bar')
        else:
            self.assertEqual(python_tools.to_cmake_path('/foo/bar'),
                             '/foo/bar')
            self.assertEqual(python_tools.from_cmake_path('/foo/bar'),
                             '/foo/bar')

    def test_get_existing_links(self):
        """Test get_existing_links()"""
        with utils.TempDir() as tmpdir:
            utils.write_file(os.path.join(tmpdir, 'orig'), 'foo')
            os.symlink('orig', os.path.join(tmpdir, 'lnk'))
            self.assertEqual(python_tools.get_existing_links(tmpdir),
                             [os.path.join(tmpdir, 'lnk')])

    def test_mkdir_no_clean_file(self):
        """Test mkdir(clean=False) with an existing file in the way"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, 'orig')
            utils.write_file(fname, 'foo')
            python_tools.mkdir(fname, clean=False)
            self.assertTrue(os.path.exists(fname))
            self.assertTrue(os.path.isdir(fname))

    def test_mkdir_no_clean_dir(self):
        """Test mkdir(clean=False) with an existing dir in the way"""
        with utils.TempDir() as tmpdir:
            path = os.path.join(tmpdir, 'path')
            os.mkdir(path)
            fname, ok_link, bad_link = [os.path.join(path, x)
                                        for x in ['orig', 'ok_lnk', 'bad_lnk']]
            utils.write_file(fname, 'foo')
            os.symlink(fname, ok_link)
            os.symlink('/not/exist', bad_link)
            python_tools.mkdir(path, clean=False)
            self.assertTrue(os.path.exists(fname))
            self.assertTrue(os.path.lexists(ok_link))
            # Broken link should have been cleaned
            self.assertFalse(os.path.lexists(bad_link))

    def test_mkdir_clean_dir(self):
        """Test mkdir(clean=True) with an existing dir in the way"""
        with utils.TempDir() as tmpdir:
            path = os.path.join(tmpdir, 'path')
            os.mkdir(path)
            fname, pytest, ok_link, bad_link = [os.path.join(path, x)
                         for x in ['orig', 'foo.pytests', 'ok_lnk', 'bad_lnk']]
            utils.write_file(fname, 'foo')
            os.symlink(fname, ok_link)
            os.symlink('/not/exist', bad_link)
            python_tools.mkdir(path, clean=True)
            self.assertTrue(os.path.exists(fname))
            # foo.pytests should have been cleaned
            self.assertFalse(os.path.exists(pytest))
            # Both links should have been cleaned
            self.assertFalse(os.path.lexists(ok_link))
            self.assertFalse(os.path.lexists(bad_link))

    def test_mkdir(self):
        """Test mkdir()"""
        with utils.TempDir() as tmpdir:
            path = os.path.join(tmpdir, 'foo', 'bar', 'baz')
            python_tools.mkdir(path, clean=True)
            self.assertTrue(os.path.exists(path))
            self.assertTrue(os.path.isdir(path))

    def test_quote(self):
        """Test quote()"""
        in_str = r'test string 1\f 2\b 3 \ "foo" 4'
        out_str = r'test string 1 2 3 \\ \"foo\" 4'
        self.assertEqual(python_tools.quote(in_str), out_str)

    def test_get_glob(self):
        """Test get_glob()"""
        with utils.TempDir() as tmpdir:
            for fname in ['a.1', 'b.1', 'c.2', 'd.2', 'e.3', 'f.3']:
                utils.write_file(os.path.join(tmpdir, fname), 'foo')
            g = python_tools.get_glob([os.path.join(tmpdir, x)
                                       for x in ['*.3', '*.2']])
            self.assertEqual(g, [os.path.join(tmpdir, x)
                                 for x in ['e.3', 'f.3', 'c.2', 'd.2']])

if __name__ == '__main__':
    unittest.main()
