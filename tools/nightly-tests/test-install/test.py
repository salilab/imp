import IMP
import IMP.core
import IMP.example
import IMP.atom
import IMP.restrainer
import IMP.container
import unittest
import os

class IMPInstallTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check install of basic set of modules"""
        IMP.base.set_log_level(IMP.base.VERBOSE)

        m = IMP.Model()

        p0 = IMP.Particle(m)
        d0 = IMP.core.XYZ.setup_particle(p0)
        d0.set_coordinates(IMP.algebra.Vector3D(0,0,2))

        r0 = IMP.example.ExampleRestraint(p0, 1)
        m.add_restraint(r0)

    def test_data_installed(self):
        """Check install of data files"""
        d = IMP.atom.get_data_path("top_heav.lib")
        self.assert_(os.path.exists(d))

    def test_examples_installed(self):
        """Check install of example files"""
        d = IMP.restrainer.get_example_path("em_restraint.py")
        self.assert_(os.path.exists(d))

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
