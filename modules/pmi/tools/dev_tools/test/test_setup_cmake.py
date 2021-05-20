import unittest
import subprocess
import os
import utils
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
setup_cmake = os.path.join(TOPDIR, 'setup_cmake.py')
sys.path.insert(0, TOPDIR)
import setup_cmake as setup_cmake_module
del sys.path[0]


class Tests(unittest.TestCase):

    def test_get_files(self):
        """Test get_files() function"""
        with utils.TempDir() as tmpdir:
            os.mkdir(os.path.join(tmpdir, 'subdir'))
            for f in [os.path.join(tmpdir, 'subdir', x)
                      for x in ['foo.cpp', 'bar.c', '_baz.cpp']]:
                utils.write_file(f, "foo")
            f = setup_cmake_module._get_files(tmpdir, '.cpp')
            self.assertEqual(f, ['subdir/foo.cpp'])

    def test_make_files_no_json(self):
        """Test make_files() function, no JSON"""
        with utils.TempDir() as tmpdir:
            for f in [os.path.join(tmpdir, x)
                      for x in ['foo.cpp', 'bar.cu', 'baz.py']]:
                utils.write_file(f, "foo")
            setup_cmake_module.make_files(tmpdir)
            self.assertEqual(utils.read_file(os.path.join(tmpdir,
                                                          'Files.cmake')),
                             'set(pyfiles "baz.py")\nset(cppfiles "foo.cpp")\n'
                             'set(cudafiles "bar.cu")\n')

    def test_make_files_with_json(self):
        """Test make_files() function, with JSON"""
        with utils.TempDir() as tmpdir:
            for f in [os.path.join(tmpdir, x)
                      for x in ['foo.cpp', 'bar.cu', 'baz.py', 'foo.json']]:
                utils.write_file(f, "foo")
            setup_cmake_module.make_files(tmpdir)
            self.assertEqual(utils.read_file(os.path.join(tmpdir,
                                                          'Files.cmake')),
                             'set(pyfiles "baz.py")\nset(cppfiles "foo.cpp")\n'
                             'set(cudafiles "bar.cu")\n'
                             'set(jsonfiles "foo.json")\n')

    def test_setup_cmake(self):
        """Test setup_cmake.py"""
        subdirs = ['src', '.foo', 'gitflow', os.path.join('src', 'internal'),
                   'bar', os.path.join('bar', 'test')]
        with utils.TempDir() as tmpdir:
            for s in subdirs:
                os.mkdir(os.path.join(tmpdir, s))
                utils.write_file(os.path.join(tmpdir, s, 'foo.py'), "foo")

            p = subprocess.Popen([setup_cmake], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)

            fname = os.path.join(tmpdir, 'src', 'Files.cmake')
            f = utils.read_file(fname)
            self.assertEqual(f.split('\n')[0],
                             'set(pyfiles "foo.py;internal/foo.py")')
            os.unlink(fname)

            fname = os.path.join(tmpdir, 'bar', 'test', 'Files.cmake')
            f = utils.read_file(fname)
            self.assertEqual(f.split('\n')[0],
                             'set(pyfiles "foo.py")')
            os.unlink(fname)
            # Assert that no other files were created (rmdirs would fail)
            for s in subdirs[::-1]:
                os.unlink(os.path.join(tmpdir, s, 'foo.py'))
                os.rmdir(os.path.join(tmpdir, s))
            self.assertEqual(os.listdir(tmpdir), [])


if __name__ == '__main__':
    unittest.main()
