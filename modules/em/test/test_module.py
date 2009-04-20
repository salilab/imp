import unittest
import IMP
import IMP.test


class ModuleTest(IMP.test.TestCase):
    """Class to test various low-level IMP module things"""

    def test_exceptions(self):
        """Make sure that exceptions work correctly"""
        # Using SWIG-generated std::vector classes (such as IMP.Particles)
        # can cause a crash due to an unhandled exception. This code crashes
        # with swig 1.3.33 if more than one module defines vector wrappers.
        # See SWIG bug #1863647 at
        # http://sf.net/tracker/?func=detail&atid=101645&aid=1863647&group_id=1645
        # For now (r675) our workaround is to #define PySwigIterator to a unique
        # name in each module that uses vector classes, in the SWIG .i file.
        for p in IMP.Particles():
            print p

if __name__ == '__main__':
    unittest.main()
