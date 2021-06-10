import IMP
import IMP.core
import IMP.example
import IMP.atom
import IMP.container
import unittest
import subprocess
import os
import sys


def find_path(exe):
    """Find `exe` in the PATH and return the fully-qualified path to it."""
    path_comps = os.environ['PATH'].split(os.pathsep)
    for c in path_comps:
        p = os.path.join(c, exe)
        if os.path.exists(p):
            return p
    return exe


class App(object):
    def __init__(self, name, python=False, can_exit_1=False):
        self.name = name
        self.python = python
        # Some apps can exit with 1 (missing Python modules) as well as 0
        self.can_exit_1 = can_exit_1

    def test(self):
        """Run the app and return the exit code and any output."""
        args = [self.name, '--help']
        if sys.platform == 'win32' and self.python:
            # Our WINE test environment doesn't seem to currently support
            # running Python applications via the .exe wrappers, so find them
            # in the PATH and run via python.exe instead
            args = [sys.executable, find_path(self.name + '.py'), '--help']
        try:
            p = subprocess.Popen(args, stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT)
        except OSError:
            raise OSError("Could not run %s" % self.name)
        out = p.stdout.read()
        p.stdout.close()
        return p.wait(), out


class IMPInstallTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check install of basic set of modules"""
        IMP.set_log_level(IMP.VERBOSE)

        m = IMP.Model()

        p0 = IMP.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 2))

        _ = IMP.example.ExampleRestraint(m, p0.get_index(), 1)

    def test_data_installed(self):
        """Check install of data files"""
        d = IMP.atom.get_data_path("top_heav.lib")
        self.assertTrue(os.path.exists(d))

    def test_examples_installed(self):
        """Check install of example files"""
        d = IMP.atom.get_example_path("assess_dope.py")
        self.assertTrue(os.path.exists(d))

    def test_applications_installed(self):
        """Check install of basic applications"""
        apps = [App('cnmultifit', python=True), App('foxs'),
                App('ligand_score'), App('multifit', python=True),
                App('pdb_check'), App('pdb_rmf'),
                App('rmf_cat', can_exit_1=True),
                App('saxs_merge', python=True, can_exit_1=True)]

        for app in apps:
            ret, out = app.test()
            if app.can_exit_1:
                self.assertTrue(ret == 1 or ret == 0,
                                "Return code for %s app is %d, not 0 or 1; "
                                "output is %s" % (app.name, ret, out))
            else:
                self.assertEqual(ret, 0,
                                 "Return code for %s app is %d, not 0; "
                                 "output is %s" % (app.name, ret, out))

    def test_get_numpy(self):
        """Test numpy support"""
        m1 = IMP.Model("numpy get_ints")
        p1 = IMP.Particle(m1)
        p2 = IMP.Particle(m1)
        p3 = IMP.Particle(m1)

        m2 = IMP.Model("numpy no get_ints")
        p12 = IMP.Particle(m2)

        k = IMP.IntKey("myf")
        p1.add_attribute(k, 1)
        p2.add_attribute(k, 2)

        if IMP.IMP_KERNEL_HAS_NUMPY:
            n = m1._get_ints_numpy(k)
            self.assertIs(n.base, m1)
            self.assertEqual(len(n), 2) # no int attribute for p3
            self.assertEqual(n[0], 1)
            self.assertEqual(n[1], 2)
            n[0] = 42
            n[1] = 24
            self.assertEqual(p1.get_value(k), 42)
            self.assertEqual(p2.get_value(k), 24)

            n = m2._get_ints_numpy(k)
            self.assertIs(n.base, m2)
            self.assertEqual(len(n), 0) # no int key for this model
        else:
            self.assertRaises(NotImplementedError, m1._get_ints_numpy, k)


if __name__ == '__main__':
    unittest.main()
