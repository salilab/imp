import IMP.test
import IMP
import IMP.statistics
import IMP.algebra
import random

def show_2d(h):
    import numpy as np
    import matplotlib.cm as cm
    import matplotlib.mlab as mlab
    import matplotlib.pyplot as plt
    cg= h.get_counts()
    steps=cg.get_unit_cell()
    x = np.arange(cg.get_bounding_box().get_corner(0)[0]+.5*steps[0],
                  cg.get_bounding_box().get_corner(1)[0]-.5*steps[0])
    y = np.arange(cg.get_bounding_box().get_corner(0)[1]+.5*steps[1],
                  cg.get_bounding_box().get_corner(1)[1]-.5*steps[1])
    X, Y = np.meshgrid(x, y)
    Z,junk= np.meshgrid(x,y)
    for i,xi in enumerate(x):
        for j, yj in enumerate(y):
            Z[i][j]=cg[cg.get_nearest_index(IMP.algebra.Vector2D(xi,yj))]
    im = plt.pcolor(X,Y, Z, cmap=cm.jet)
    plt.colorbar(im)
    plt.show()

class Tests(IMP.test.TestCase):
    "Test that the histogram functionality works"
    def Setup(self):
        IMP.test.TestCase.setUp(self)
    def test_histogram_1d(self):
        """Check that the histogram1D of a gaussian is OK"""
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

    def test_histogram_2d(self):
        """Check that the histogram1D of a gaussian is OK"""
        hist = IMP.statistics.Histogram2D(5.0, IMP.algebra.BoundingBox2D(IMP.algebra.Vector2D(-100, -100),
                                                                IMP.algebra.Vector2D( 100, 100)))
        sigma=10
        for i in range(10000):

            x=random.gauss(0, 10)
            y=random.gauss(0, 10)
            #print x
            hist.add(IMP.algebra.Vector2D(x, y))
        print hist.get_counts()
        print hist.get_frequencies()
        #show_2d(hist)

if __name__ == '__main__':
    IMP.test.main()
