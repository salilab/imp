import IMP
import IMP.test
from IMP.kmeans import KMeansWrapper

class KMeansTest(IMP.test.TestCase):
    def  test_simple_kmeans(self):
        km = KMeansWrapper()
        # add data
        km.addDataPt([1, 2])
        km.addDataPt([10, 12])
        km.addDataPt([0, -1])
        km.addDataPt([11, 13])
        km.addDataPt([0, 0])
        km.addDataPt([1, 0])
        km.addDataPt([9.4, 10])
        km.addDataPt([9.2, 11])
        km.addDataPt([0, 0])
        self.assertEqual(km.getDataPoint(0)[0], 1)
        self.assertEqual(km.getDataPoint(0)[1], 2)
        # print data
        n = km.getNDataPoints()
        print "Number of points: ", n
        self.assertEqual(n,9)
        for i in range(0,n):
            print i
            km.getDataPoint(i)
        # cluster data
        k = 2
        km.execute(k)
        # print centers one by one
        print "\nCenters:"
        nCenters = km.getNCenters() # this should actually be equal to k
        print "Number of centers: ", nCenters
        self.assertEqual(nCenters, k)
        print range(1, nCenters)
        for i in range(0, nCenters):
            print "Center ", i, "   ", km.getCenter(i)
        # make sure centers were found correctly
        c0 = km.getCenter(0)
        c1 = km.getCenter(1)
        if(c0[0] < 3.0):
            self.assertAlmostEqual(c0[0],0.4,1)
            self.assertAlmostEqual(c0[1],0.2,1)
        else:
            self.assertAlmostEqual(c0[0],9.9,1)
            self.assertAlmostEqual(c0[1],11.5,1)


if __name__ == '__main__':
    IMP.test.main()
