import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):

    def test_mismatched_dimensions(self):
        """Map arithmetic should reject maps of different dimensions"""
        big = IMP.em.create_density_map(10, 10, 10, 1.0)
        small = IMP.em.create_density_map(2, 2, 2, 1.0)
        big.calcRMS()
        small.calcRMS()
        self.assertRaisesUsageException(big.pick_max, small)
        self.assertRaisesUsageException(IMP.em.multiply, big, small)
        self.assertRaisesUsageException(
            IMP.em.CoarseConvolution.convolution, big, small, 0.0)


if __name__ == '__main__':
    IMP.test.main()
