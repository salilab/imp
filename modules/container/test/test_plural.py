import unittest
import IMP.container
import IMP.test

plural_exceptions=[]

show_exceptions=[]

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test all container objects have plural version"""
        return self.assertPlural(IMP.container, plural_exceptions)

    def test_2(self):
        """Test all container objects have show"""
        return self.assertShow(IMP.container, show_exceptions)

if __name__ == '__main__':
    unittest.main()
