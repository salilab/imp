import IMP
import IMP.test

@IMP.deprecated_function("2.1", "Use my_new_function(args) instead")
def my_deprecated_function():
    pass

@IMP.deprecated_object("2.1", "Use MyNewClass instead")
class MyClass(object):
    pass

class TestClass(object):
    @IMP.deprecated_method("2.1", "Use my_new_method(args) instead")
    def my_deprecated_method(self):
        pass

class Tests(IMP.test.TestCase):
    def test_deprecated_function(self):
        """Test deprecated_function decorator"""
        with IMP.allow_deprecated(False):
            self.assertRaises(IMP.UsageException, my_deprecated_function)

    def test_deprecated_object(self):
        """Test deprecated_object decorator"""
        with IMP.allow_deprecated(False):
            self.assertRaises(IMP.UsageException, MyClass)

    def test_deprecated_method(self):
        """Test deprecated_method decorator"""
        with IMP.allow_deprecated(False):
            t = TestClass()
            self.assertRaises(IMP.UsageException, t.my_deprecated_method)

    def test_deprecated_module(self):
        """Test deprecated_module function"""
        with IMP.allow_deprecated(False):
            self.assertRaises(IMP.UsageException, __import__,
                              'deprecated_module')

if __name__ == '__main__':
    IMP.test.main()
