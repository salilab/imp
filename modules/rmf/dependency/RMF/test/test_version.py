import RMF
import unittest


class Tests(unittest.TestCase):

    def test_version(self):
        """Test RMF.__version__"""
        self.assertIn('.', RMF.__version__)


if __name__ == '__main__':
    unittest.main()
