import unittest
import subprocess
import os
import utils

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
cleanup_pycs = os.path.join(TOPDIR, 'cleanup_pycs.py')

class Tests(unittest.TestCase):

    def test_cleanup_pycs(self):
        """Test cleanup_pycs.py script"""
        with utils.TempDir() as tmpdir:
            ok_py = os.path.join(tmpdir, "ok.py")
            ok_pyc = os.path.join(tmpdir, "ok.pyc")
            to_delete_pyc = os.path.join(tmpdir, "del.pyc")
            for f in (ok_py, ok_pyc, to_delete_pyc):
                utils.write_file(f, "")
            p = subprocess.Popen([cleanup_pycs, tmpdir])
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)
            self.assertTrue(os.path.exists(ok_py))
            self.assertTrue(os.path.exists(ok_pyc))
            self.assertFalse(os.path.exists(to_delete_pyc))

    def test_cleanup_pycs_cwd(self):
        """Test cleanup_pycs.py script in cwd"""
        with utils.TempDir() as tmpdir:
            ok_py = os.path.join(tmpdir, "ok.py")
            ok_pyc = os.path.join(tmpdir, "ok.pyc")
            to_delete_pyc = os.path.join(tmpdir, "del.pyc")
            to_delete2_pyc = os.path.join(tmpdir, "del2.pyc")
            for f in (ok_py, ok_pyc, to_delete_pyc, to_delete2_pyc):
                utils.write_file(f, "")
            p = subprocess.Popen([cleanup_pycs], cwd=tmpdir)
            stdout, stderr = p.communicate()
            self.assertEqual(p.returncode, 0)
            self.assertTrue(os.path.exists(ok_py))
            self.assertTrue(os.path.exists(ok_pyc))
            self.assertFalse(os.path.exists(to_delete_pyc))
            self.assertFalse(os.path.exists(to_delete2_pyc))

if __name__ == '__main__':
    unittest.main()
