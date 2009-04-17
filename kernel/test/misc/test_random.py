import unittest
import IMP
import IMP.test

class RandomTests(IMP.test.TestCase):
    def test_random_number_generator(self):
        """Test the random number generator"""
        IMP.random_number_generator.seed(24)
        a1 = IMP.random_number_generator()
        a2 = IMP.random_number_generator()
        self.assertNotEqual(a1, a2)
        IMP.random_number_generator.seed(24)
        a3 = IMP.random_number_generator()
        self.assertEqual(a1, a3)

if __name__ == '__main__':
    unittest.main()
