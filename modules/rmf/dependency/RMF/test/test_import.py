import unittest


class GenericTest(unittest.TestCase):

    def test_data_types(self):
        """Test that import works"""
        import sys
        print("path", sys.path)
        import RMF  # noqa: F401


if __name__ == '__main__':
    unittest.main()
