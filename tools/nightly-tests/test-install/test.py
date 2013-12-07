import IMP
import IMP.core
import IMP.example
import IMP.atom
import IMP.restrainer
import IMP.container
import unittest
import subprocess
import os


class IMPInstallTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check install of basic set of modules"""
        IMP.base.set_log_level(IMP.base.VERBOSE)

        m = IMP.kernel.Model()

        p0 = IMP.kernel.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d0.set_coordinates(IMP.algebra.Vector3D(0, 0, 2))

        r0 = IMP.example.ExampleRestraint(m, p0.get_index(), 1)

    def test_data_installed(self):
        """Check install of data files"""
        d = IMP.atom.get_data_path("top_heav.lib")
        self.assert_(os.path.exists(d))

    def test_examples_installed(self):
        """Check install of example files"""
        d = IMP.restrainer.get_example_path("em_restraint.py")
        self.assert_(os.path.exists(d))

    def test_applications_installed(self):
        """Check install of basic applications"""
        apps = ['cnmultifit.py', 'foxs', 'idock.py', 'ligand_score',
                'multifit.py', 'pdb_check', 'pdb_rmf', 'rmf_cat',
                'saxs_merge.py']
        for app in apps:
            try:
                p = subprocess.Popen([app, '--help'], stdout=subprocess.PIPE,
                                     stderr=subprocess.STDOUT)
            except OSError:
                raise OSError("Could not run %s" % app)
            out = p.stdout.read()
            ret = p.wait()
            self.assert_(ret == 1 or ret == 0,
                         "Return code for %s app is %d, not 0 or 1; "
                         "output is %s" % (app, ret, out))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
