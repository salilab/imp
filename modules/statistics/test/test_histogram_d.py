import IMP.test
import IMP
import IMP.statistics
import IMP.core
import IMP.algebra
import random
class HistogramTests(IMP.test.TestCase):
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

if __name__ == '__main__':
    IMP.test.main()
