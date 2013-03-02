import IMP.base
import IMP.test
import os.path

class Tests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test the get_data_directory function"""
        f = IMP.base.get_data_path("README")
        self.assertTrue(os.path.exists(f) and not os.path.isdir(f))

    def test_bad_data_directory(self):
        """get_data_directory should raise an error if it does not exist"""
        self.assertRaises(IMP.base.IOException, IMP.base.get_data_path, "not/exist")

if __name__ == '__main__':
    IMP.test.main()
