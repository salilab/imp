import IMP.test
import IMP
import IMP.statistics
import IMP.algebra
import random


class HistogramTests(IMP.test.TestCase):
    "Test that the histogram functionality works"
    def Setup(self):
        IMP.test.TestCase.setUp(self)
    def test_histogram_construction(self):
        """Check that the histogramD of a gaussian is OK"""
        hist = IMP.statistics.Histogram1D(5.0, IMP.algebra.BoundingBox1D(IMP.algebra.Vector1D(-100),
                                                                IMP.algebra.Vector1D( 100)))
        sigma=10
        for i in range(10000):
            x=random.gauss(0, 10)
            #print x
            hist.add(IMP.algebra.Vector1D(x))
        print hist.get_counts()
        print hist.get_frequencies()
        IMP.statistics.show_histogram(hist)

if __name__ == '__main__':
    IMP.test.main()
