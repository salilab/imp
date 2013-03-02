import IMP.test
import IMP
import IMP.statistics
import IMP.algebra
import random
class Tests(IMP.test.TestCase):
    "Test that the histogram functionality works"
    def Setup(self):
        IMP.test.TestCase.setUp(self)
    def test_histogram_construction(self):
        """Check that the histogram is constructed correctly"""
        start=0
        end=10
        num_bins=100
        hist = IMP.statistics.Histogram(start,end,num_bins)
        for i in range(100):
            hist.add(random.uniform(start,end))
        self.assertEqual(hist.get_total_count(),100)
    def test_histogram_construction2(self):
        """Check that the uniform histogram is constructed correctly"""
        start=0
        end=10
        num_bins=100
        hist = IMP.statistics.Histogram(start,end,num_bins)
        for i in range(100):
            hist.add(0)
        self.assertEqual(hist.get_top(0.9),0)

    def test_get_top(self):
        """Test that histogram top works"""
        start=0
        end=10
        num_bins=100
        hist = IMP.statistics.Histogram(start,end,num_bins)
        for i in range(50):
            hist.add(random.uniform(start,7))
        for i in range(40):
            hist.add(random.uniform(7,8.9))
        for i in range(10):
            hist.add(random.uniform(9,end))
        self.assertLess(hist.get_top(0.49), 7.1)
        self.assertLess(hist.get_top(0.89), 9.1)
        self.assertGreater(hist.get_top(0.99), 9)

if __name__ == '__main__':
    IMP.test.main()
