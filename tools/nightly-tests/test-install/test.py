import IMP
import IMP.core
import IMP.example
import IMP.atom
import IMP.container
import unittest
import subprocess
import os


class IMPInstallTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check install of basic set of modules"""
        IMP.set_log_level(IMP.VERBOSE)

        m = IMP.kernel.Model()

        p0 = IMP.kernel.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 2))

        r0 = IMP.example.ExampleRestraint(m, p0.get_index(), 1)

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
        # These apps should *always* exit with code 0
        zero_apps = ['cnmultifit', 'foxs', 'ligand_score',
                     'multifit', 'pdb_check', 'pdb_rmf']
        # These apps may also exit with 1 (missing Python modules)
        one_apps = ['rmf_cat', 'saxs_merge']
        def test_app(app):
            try:
                p = subprocess.Popen([app, '--help'], stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT)
            except OSError:
                raise OSError("Could not run %s" % app)
            return p.wait(), p.stdout.read()
        for app in zero_apps:
            ret, out = test_app(app)
            self.assertEqual(ret, 0,
                             "Return code for %s app is %d, not 0; "
                             "output is %s" % (app, ret, out))
        for app in one_apps:
            ret, out = test_app(app)
            self.assertTrue(ret == 1 or ret == 0,
                            "Return code for %s app is %d, not 0 or 1; "
                            "output is %s" % (app, ret, out))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
