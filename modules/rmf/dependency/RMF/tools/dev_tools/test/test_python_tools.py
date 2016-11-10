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
            utils.write_file(pytest, 'foo')
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

    def test_rewrite_exists(self):
        """Test rewrite() with an existing file"""
        with utils.TempDir() as tmpdir:
            fname = os.path.join(tmpdir, 'fname')
            utils.write_file(fname, 'foo')
            python_tools.rewrite(fname, 'foo')
            self.assertEqual(utils.read_file(fname), 'foo')
            python_tools.rewrite(fname, 'bar')
            self.assertEqual(utils.read_file(fname), 'bar')
            python_tools.rewrite(fname, 'foo', verbose=True)
            self.assertEqual(utils.read_file(fname), 'foo')

    def test_rmdir(self):
        """Test rmdir()"""
        with utils.TempDir() as tmpdir:
            subdir = os.path.join(tmpdir, 'subdir')
            os.mkdir(subdir)
            self.assertTrue(os.path.exists(subdir))
            python_tools.rmdir(subdir)
            self.assertFalse(os.path.exists(subdir))
            # Not an error to rmdir a non-existent location
            python_tools.rmdir('/not/exist')

    def test_link_no_source(self):
        """Test link() with nonexistent source"""
        for verbose in (True, False):
            # no-op if source does not exist
            python_tools.link('/not/exist', '/other/not/exist', verbose=verbose)

    def test_link_already_linked(self):
        """Test link() with target already linked to source"""
        with utils.TempDir() as tmpdir:
            s = os.path.join(tmpdir, 'source')
            t = os.path.join(tmpdir, 'target')
            utils.write_file(s, 'foo')
            os.symlink(s, t)
            # no-op
            python_tools.link(s, t)

    def test_link_already_linked_other(self):
        """Test link() with target already linked to other file"""
        with utils.TempDir() as tmpdir:
            s = os.path.join(tmpdir, 'source')
            o = os.path.join(tmpdir, 'other')
            t = os.path.join(tmpdir, 'target')
            utils.write_file(s, 'foo')
            utils.write_file(o, 'foo')
            os.symlink(o, t)
            self.assertEqual(os.readlink(t), o)
            python_tools.link(s, t)
            self.assertEqual(os.readlink(t), s)

    def test_link_dir_exists(self):
        """Test link() with target an existing directory"""
        with utils.TempDir() as tmpdir:
            s = os.path.join(tmpdir, 'source')
            t = os.path.join(tmpdir, 'target')
            utils.write_file(s, 'foo')
            os.mkdir(t)
            utils.write_file(os.path.join(t, 'foo'), 'foo')
            python_tools.link(s, t)
            self.assertEqual(os.readlink(t), s)

    def test_link_file_exists(self):
        """Test link() with target an existing file"""
        with utils.TempDir() as tmpdir:
            s = os.path.join(tmpdir, 'source')
            t = os.path.join(tmpdir, 'target')
            utils.write_file(s, 'foo')
            utils.write_file(t, 'foo')
            python_tools.link(s, t, verbose=True)
            self.assertEqual(os.readlink(t), s)

    def test_get_project_info(self):
        """Test get_project_info()"""
        with utils.TempDir() as tmpdir:
            subdir = os.path.join(tmpdir, 'subdir')
            os.mkdir(subdir)
            fname = os.path.join(tmpdir, '.imp_info.py')
            utils.write_file(fname, '{\n"name": "foo"\n}\n')
            for d in subdir, tmpdir:
                self.assertEqual(python_tools.get_project_info(d),
                                 {'name':'foo'})
            self.assertRaises(ValueError, python_tools.get_project_info,
                              '/not/exist')

    def test_get_modules(self):
        """Test get_modules()"""
        with utils.TempDir() as tmpdir:
            subdir = os.path.join(tmpdir, 'modules')
            os.mkdir(subdir)
            mods = [os.path.join(subdir, x) for x in ['a', 'b', 'c']]
            for m in mods:
                os.mkdir(m)
            utils.write_file(os.path.join(mods[0], 'dependencies.py'), 'f')
            self.assertEqual(python_tools.get_modules(tmpdir),
                             [('a', mods[0])])

    def test_split(self):
        """Test split()"""
        self.assertEqual(python_tools.split("foo:bar:baz"),
                         ["foo", "bar", "baz"])
        self.assertEqual(python_tools.split("foo::baz"),
                         ["foo", "baz"])
        self.assertEqual(python_tools.split("foo\:bar:b@z"),
                         ["foo:bar", "b:z"])

    def test_link_dir(self):
        """Test link_dir()"""
        with utils.TempDir() as tmpdir:
            source = os.path.join(tmpdir, 'source')
            os.mkdir(source)
            utils.write_file(os.path.join(source, 'foo'), 'foo')
            utils.write_file(os.path.join(source, 'baz'), 'foo')
            utils.write_file(os.path.join(source, 'CMakeLists.txt'), 'foo')
            target = os.path.join(tmpdir, 'target')
            os.mkdir(target)
            # Add an OK existing symlink
            os.symlink(os.path.join(source, 'foo'),
                       os.path.join(target, 'foo'))
            # Add a bad existing symlink
            os.symlink(os.path.join(source, 'bar'),
                       os.path.join(target, 'badln'))
            # Add a good existing symlink
            os.symlink(os.path.join(source, 'foo'),
                       os.path.join(target, 'goodln'))
            python_tools.link_dir(source, target)
            self.assertRaises(TypeError, python_tools.link_dir,
                              source, target, match='foo')
            # all links should have been removed
            self.assertEqual(sorted(os.listdir(target)), sorted(['foo', 'baz']))

    def test_link_dir_no_clean(self):
        """Test link_dir() with clean=False"""
        with utils.TempDir() as tmpdir:
            source = os.path.join(tmpdir, 'source')
            os.mkdir(source)
            utils.write_file(os.path.join(source, 'foo'), 'foo')
            utils.write_file(os.path.join(source, 'baz'), 'foo')
            utils.write_file(os.path.join(source, 'CMakeLists.txt'), 'foo')
            target = os.path.join(tmpdir, 'target')
            os.mkdir(target)
            # Add an OK existing symlink
            os.symlink(os.path.join(source, 'foo'),
                       os.path.join(target, 'foo'))
            # Add a bad existing symlink
            os.symlink(os.path.join(source, 'bar'),
                       os.path.join(target, 'badln'))
            # Add a good existing symlink
            os.symlink(os.path.join(source, 'foo'),
                       os.path.join(target, 'goodln'))
            python_tools.link_dir(source, target, clean=False)
            # bad link should have been removed, but not the good link
            self.assertEqual(sorted(os.listdir(target)),
                             sorted(['foo', 'baz', 'goodln']))

if __name__ == '__main__':
    unittest.main()
