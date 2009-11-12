import unittest
import IMP
import IMP.test
import os.path

class ExceptionTests(IMP.test.TestCase):
    """Check IMP exceptions"""

    def assertSubclass(self, derived, base):
        self.assert_(issubclass(derived, base),
                     "%s is not a subclass of %s" % (derived, base))

    def test_hierarchy(self):
        """Test class hierarchy of IMP exceptions"""
        self.assertSubclass(IMP.Exception, Exception)
        # Make sure that all exceptions derive from IMP.Exception
        for subclass in (IMP.InternalException, IMP.ModelException,
                         IMP.UsageException, IMP.IndexException,
                         IMP.IOException, IMP.ValueException):
            self.assertSubclass(subclass, IMP.Exception)
        # Check for exceptions that also should derive from Python types
        self.assertSubclass(IMP.IndexException, IndexError)
        self.assertSubclass(IMP.IOException, IOError)
        self.assertSubclass(IMP.ValueException, ValueError)

if __name__ == '__main__':
    unittest.main()
