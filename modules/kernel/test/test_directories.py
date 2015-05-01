from __future__ import print_function
import IMP.test
import os.path


class Tests(IMP.test.TestCase):

    def test_bad_data_directory(self):
        """get_data_directory should raise an error if it does not exist"""
        print("HI")
        self.assertRaises(
            IMP.IOException,
            IMP.get_data_path,
            "not/exist")
        print("HO")

if __name__ == '__main__':
    IMP.set_deprecation_exceptions(True)
    IMP.test.main()
