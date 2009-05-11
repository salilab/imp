import unittest
import IMP.test
import IMP.statistics

class RandomTests(IMP.test.TestCase):
    """Test random distributions"""

    def test_random_gauss(self):
        """Test random Gaussian distribution"""
        x = IMP.statistics.random_gauss()
        y = IMP.statistics.random_gauss()
        # Strictly speaking, x *could* equal y, but it almost never *should*...
        self.assertNotEqual(x, y)

if __name__ == '__main__':
    unittest.main()
