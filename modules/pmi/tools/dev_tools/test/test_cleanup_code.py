import unittest
import subprocess
import os
import utils

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
cleanup_py = os.path.join(TOPDIR, 'cleanup_code.py')


class Tests(unittest.TestCase):

    def make_dummy_format(self, name, d, exitval=0):
        """Make a dummy formmater binary in the directory d.
           Return the modified environment needed to put this binary
           in the PATH."""
        fname = os.path.join(d, name)
        utils.write_file(fname, "#!/bin/sh\necho foo\nexit %d\n" % exitval)
        os.chmod(fname, 493)  # 493 = 0755
        env = os.environ.copy()
        env['PATH'] = d + os.pathsep + env['PATH']
        return env

    def test_python_reindent(self):
        """Test cleanup_code script on Python files with reindent."""
        for args in ['--all', 'test.py']:
            with utils.TempDir() as tmpdir:
                pyfile = os.path.join(tmpdir, 'test.py')
                utils.write_file(pyfile, 'def foo():\n  bar\n')
                p = subprocess.Popen([cleanup_py, args], cwd=tmpdir)
                stdout, stderr = p.communicate()
                self.assertEqual(p.returncode, 0)
                # 2-space indentation should have been corrected to 4-space
                self.assertEqual(utils.read_file(pyfile),
                                 'def foo():\n    bar\n')

    def test_python_autopep8(self):
        """Test cleanup_code script on Python files with autopep8."""
        for args in ['--all', 'test.py']:
            with utils.TempDir() as tmpdir:
                env = self.make_dummy_format('autopep8', tmpdir)
                pyfile = os.path.join(tmpdir, 'test.py')
                utils.write_file(pyfile, 'def foo():\n  bar\n')
                p = subprocess.Popen([cleanup_py, '-a', '-v', args],
                                     cwd=tmpdir, env=env)
                stdout, stderr = p.communicate()
                self.assertEqual(p.returncode, 0)
                # dummy autopep8 should have written out 'foo'
                self.assertEqual(utils.read_file(pyfile), 'foo\n')

    def test_cpp_clang(self):
        """Test cleanup_code script on C++ files with clang-format."""
        # directories that should be ignored
        igdirs = ['dependency', 'eigen3', 'igdir', 'git-repo']
        for args in ['--all', 'test.cpp']:
            with utils.TempDir() as tmpdir:
                env = self.make_dummy_format('clang-format', tmpdir)
                for d in igdirs:
                    os.mkdir(os.path.join(tmpdir, d))
                    utils.write_file(os.path.join(tmpdir, d, 'test.h'), 'bar')
                # git-repo is a git submodule so shouldn't be descended into
                os.mkdir(os.path.join(tmpdir, 'git-repo', '.git'))
                cppfile = os.path.join(tmpdir, 'test.cpp')
                utils.write_file(cppfile, 'bar')
                p = subprocess.Popen([cleanup_py, '-v', '-e', 'igdir', args],
                                     cwd=tmpdir, env=env)
                stdout, stderr = p.communicate()
                self.assertEqual(p.returncode, 0)
                # dummy clang-format should have written out 'foo'
                self.assertEqual(utils.read_file(cppfile), 'foo\n')
                # ignored directories should be unchanged
                for d in igdirs:
                    con = utils.read_file(os.path.join(tmpdir, d, 'test.h'))
                    self.assertEqual(con, 'bar')

    def test_no_files(self):
        """Test cleanup_code script with no files selected."""
        with utils.TempDir() as tmpdir:
            p = subprocess.Popen([cleanup_py], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 2)

    def test_run_error(self):
        """Test cleanup_code handling of subprocess error"""
        with utils.TempDir() as tmpdir:
            env = self.make_dummy_format('autopep8', tmpdir, exitval=1)
            pyfile = os.path.join(tmpdir, 'test.py')
            utils.write_file(pyfile, 'bar')
            p = subprocess.Popen([cleanup_py, '-a', '--all'], cwd=tmpdir,
                                 env=env)
            stdout, stderr = p.communicate()
            # error should be caught and not fail entire job
            self.assertEqual(p.returncode, 0)
            # file should be unchanged
            self.assertEqual(utils.read_file(pyfile), 'bar')


if __name__ == '__main__':
    unittest.main()
