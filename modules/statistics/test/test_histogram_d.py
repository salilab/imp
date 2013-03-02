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
        """Check that the histogramD of a gaussian is OK"""
        hist = IMP.statistics.Histogram2D(5.0, IMP.algebra.BoundingBox2D(IMP.algebra.Vector2D(-100, -100),
                                                                IMP.algebra.Vector2D(100, 100)))
        sigma=10
        for i in range(10000):
            hist.add(IMP.algebra.Vector2D(random.gauss(0, 10),
                                          random.gauss(0, 10)))
        mean= hist.get_mean()
        sigmaout= hist.get_standard_deviation(mean)
        print mean
        print sigmaout
        for i in range(0,2):
            self.assertAlmostEqual(mean[i], 0, delta=.5)
            self.assertAlmostEqual(sigmaout[i], 10, delta=.5)
    def test_histogram_construction_1(self):
        """Check that the histogram1D of a gaussian is OK"""
        hist = IMP.statistics.Histogram1D(5.0, IMP.algebra.BoundingBox1D(-100,100))
        sigma=10
        for i in range(10000):
            hist.add(random.gauss(0, 10))
        mean= hist.get_mean()
        sigmaout= hist.get_standard_deviation(mean)
        print mean
        print sigmaout
        self.assertAlmostEqual(mean[0], 0, delta=.5)
        self.assertAlmostEqual(sigmaout[0], 10, delta=.5)
    def test_get_top(self):
        """Test that histogram quantile works"""
        start=0
        end=10
        num_bins=100
        hist = IMP.statistics.Histogram1D(.1, IMP.algebra.BoundingBox1D(start, end))
        for i in range(50):
            hist.add(random.uniform(start,7))
        for i in range(40):
            hist.add(random.uniform(7,8.9))
        for i in range(10):
            hist.add(random.uniform(9,end))
        self.assertLess(IMP.statistics.get_quantile(hist, 0.49), 7.1)
        self.assertLess(IMP.statistics.get_quantile(hist, 0.89), 9.1)
        self.assertGreater(IMP.statistics.get_quantile(hist, 0.99), 9)


if __name__ == '__main__':
    IMP.test.main()
