import unittest
import IMP
import IMP.test
import os.path

class DirectoriesTests(IMP.test.TestCase):
    def test_data_directory(self):
        """Test the get_data_directory function"""
        f = IMP.get_data_directory()
        print f
        self.assert_(os.path.exists(f) and os.path.isdir(f))

if __name__ == '__main__':
    unittest.main()
