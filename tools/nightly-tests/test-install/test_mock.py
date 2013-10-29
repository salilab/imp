import unittest
import os

mock_config = os.environ['MOCK_CONFIG']

class IMPMockTests(unittest.TestCase):

    def test_modules_installed(self):
        """Check modules included in the RPM"""
        # RHEL systems don't include cgal, em2d; Fedora does
        if mock_config.startswith('fedora'):
            import IMP.cgal
            import IMP.em2d
        else:
            self.assertRaises(ImportError, __import__, 'IMP.cgal')
            self.assertRaises(ImportError, __import__, 'IMP.em2d')
        # The scratch module should not be included
        self.assertRaises(ImportError, __import__, 'IMP.scratch')
        # We build our own Eigen on RHEL, so everyone should have IMP.isd
        import IMP.isd
        # Check that most other modules (particularly those with many
        # dependencies) are present
        import IMP.cnmultifit
        import IMP.domino
        import IMP.em
        import IMP.gsl
        import IMP.kinematics
        import IMP.kmeans
        import IMP.misc
        import IMP.multifit
        import IMP.parallel
        import IMP.restrainer
        import IMP.rmf
        import IMP.rotamer
        import IMP.saxs
        import IMP.statistics
        import IMP.symmetry
        import IMP.test

if __name__ == '__main__':
    # Note we use unittest rather than IMP.test, since the latter requires
    # a unittest2 Python module to be installed
    unittest.main()
