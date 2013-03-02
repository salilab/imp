import IMP
import IMP.test
import os.path

class Tests(IMP.test.TestCase):
    """Check IMP exceptions"""

    def assertSubclass(self, derived, base):
        self.assertTrue(issubclass(derived, base),
                        "%s is not a subclass of %s" % (derived, base))

    def test_hierarchy(self):
        """Test class hierarchy of IMP exceptions"""
        self.assertSubclass(IMP.base.Exception, Exception)
        # Make sure that all exceptions derive from IMP.Exception
        for subclass in (IMP.base.InternalException, IMP.base.ModelException,
                         IMP.base.UsageException, IMP.base.IndexException,
                         IMP.base.IOException, IMP.base.ValueException,
                         IMP.base.EventException):
            self.assertSubclass(subclass, IMP.base.Exception)
        # Check for exceptions that also should derive from Python types
        self.assertSubclass(IMP.base.IndexException, IndexError)
        self.assertSubclass(IMP.base.IOException, IOError)
        self.assertSubclass(IMP.base.ValueException, ValueError)
        # check that they are hoisted to IMP
        IMP.IOException

if __name__ == '__main__':
    IMP.test.main()
