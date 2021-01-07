import unittest
import subprocess
import os
import utils

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
setup_git = os.path.join(TOPDIR, 'git', 'setup_git.py')


class Tests(unittest.TestCase):

    def test_bootstrap(self):
        """Test bootstrap_setup_git.py script"""
        bootstrap = os.path.join(TOPDIR, 'git', 'bootstrap_setup_git.py')
        with utils.TempDir() as tmpdir:
            # Both os.system() calls will fail in this script, but for now
            # we're simply exercising them, not checking the result
            p = subprocess.Popen([bootstrap], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)

    def make_dummy_git(self, d):
        """Make a dummy 'git' binary in the directory d.
           Return the modified environment needed to put this binary
           in the PATH."""
        git = os.path.join(d, 'git')
        utils.write_file(git, "#!/bin/sh")
        os.chmod(git, 493)  # 493 = 0755
        env = os.environ.copy()
        env['PATH'] = d + os.pathsep + env['PATH']
        return env

    def test_setup_git_global(self):
        """Test setup_git.py script in global mode"""
        with utils.TempDir() as tmpdir:
            env = self.make_dummy_git(tmpdir)
            p = subprocess.Popen([setup_git, '-g'], cwd=tmpdir, env=env)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)

    def test_setup_git_repo_no_git(self):
        """Test setup_git.py script with repo settings, not in a git root"""
        with utils.TempDir() as tmpdir:
            p = subprocess.Popen([setup_git], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 1)

    def test_setup_git_repo_no_dev_tools(self):
        """Test setup_git.py script with repo settings, no dev tools"""
        with utils.TempDir() as tmpdir:
            subprocess.check_call(['git', 'init'], cwd=tmpdir)
            p = subprocess.Popen([setup_git], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 1)

    def test_setup_git_repo(self):
        """Test setup_git.py script with repo settings"""
        with utils.TempDir() as tmpdir:
            subprocess.check_call(['git', 'init'], cwd=tmpdir)
            tools = os.path.join(tmpdir, 'tools')
            os.mkdir(tools)
            dev_tools = os.path.join(tools, 'dev_tools')
            os.symlink(TOPDIR, dev_tools)
            p = subprocess.Popen([setup_git], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)
            # should be OK to rerun the script
            p = subprocess.Popen([setup_git], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)


if __name__ == '__main__':
    unittest.main()
