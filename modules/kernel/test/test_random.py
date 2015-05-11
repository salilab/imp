from __future__ import print_function
import IMP.test
import numpy as np
import math

class Tests(IMP.test.TestCase):

    def test_random_number_generator(self):
        """Test the random number generator"""
        IMP.random_number_generator.seed(24)
        a1 = IMP.random_number_generator()
        a2 = IMP.random_number_generator()
        # In principle these values could be the same, I suppose, but it is
        # rather unlikely!
        self.assertNotEqual(a1, a2)
        # Reseeding should give a predictable sequence
        IMP.random_number_generator.seed(24)
        a3 = IMP.random_number_generator()
        a4 = IMP.random_number_generator()
        self.assertEqual(a1, a3)
        self.assertEqual(a2, a4)
        IMP.random_number_generator.seed(27)
        a5 = IMP.random_number_generator()
        a6 = IMP.random_number_generator()
        self.assertNotEqual(a1, a5)
        self.assertNotEqual(a2, a6)

    def test_cached_random_number_generator_normal(self):
        """Test the cached random number generator for normal dist"""
        R = IMP.get_random_doubles_normal(5000, 0.0, 1.0)
        npR = np.array(R)
        self.assertAlmostEqual( np.mean(npR), 0.0, delta=0.1)
        self.assertAlmostEqual( np.median(npR), 0.0, delta=0.1)
        self.assertAlmostEqual( np.std(npR), 1.0, delta=0.1)
        print("Normal(0,1) - mean/median/min/max/std:", np.mean(npR), np.median(npR), np.min(npR), np.max(npR), np.std(npR))


    def test_cached_random_number_generator_uniform(self):
        """Test the cached random number generator for uniform dist"""
        R = IMP.get_random_doubles_uniform(5000)
        npR = np.array(R)
        self.assertAlmostEqual( np.mean(npR), 0.5, delta=0.1)
        self.assertAlmostEqual( np.median(npR), 0.5, delta=0.1)
        self.assertAlmostEqual( np.min(npR), 0.04, delta=0.05)
        self.assertAlmostEqual( np.max(npR), 0.96, delta=0.05)
        self.assertAlmostEqual( np.std(npR), 1.0/math.sqrt(12.0), delta=0.1)
        print("Uniform(0,1) - mean/median/min/max/std:", np.mean(npR), np.median(npR), np.min(npR), np.max(npR), np.std(npR))


if __name__ == '__main__':
    IMP.test.main()
