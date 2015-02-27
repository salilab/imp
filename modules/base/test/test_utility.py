import IMP.base
import IMP.test

class Tests(IMP.test.TestCase):

    def test_unique_name(self):
        """Test get_unique_name()"""
        self.assertEqual(IMP.base.get_unique_name("test%1%"), "test0")
        self.assertEqual(IMP.base.get_unique_name("test%1%"), "test1")
        self.assertRaises(ValueError, IMP.base.get_unique_name, "test%2%")

if __name__ == '__main__':
    IMP.test.main()
