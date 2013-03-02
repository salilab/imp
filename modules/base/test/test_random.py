import IMP.base
import IMP.test

class Tests(IMP.test.TestCase):
    def test_random_number_generator(self):
        """Test the random number generator"""
        IMP.base.random_number_generator.seed(24)
        a1 = IMP.base.random_number_generator()
        a2 = IMP.base.random_number_generator()
        # In principle these values could be the same, I suppose, but it is
        # rather unlikely!
        self.assertNotEqual(a1, a2)
        # Reseeding should give a predictable sequence
        IMP.base.random_number_generator.seed(24)
        a3 = IMP.base.random_number_generator()
        a4 = IMP.base.random_number_generator()
        self.assertEqual(a1, a3)
        self.assertEqual(a2, a4)
        IMP.base.random_number_generator.seed(27)
        a5 = IMP.base.random_number_generator()
        a6 = IMP.base.random_number_generator()
        self.assertNotEqual(a1, a5)
        self.assertNotEqual(a2, a6)

if __name__ == '__main__':
    IMP.test.main()
