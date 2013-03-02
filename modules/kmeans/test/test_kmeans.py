import IMP
import IMP.test
from IMP.kmeans import KMeans

class Tests(IMP.test.TestCase):
    def  test_simple_kmeans(self):
        km = KMeans()
        # add data
        km.add_data_pt([1, 2])
        km.add_data_pt([10, 12])
        km.add_data_pt([0, -1])
        km.add_data_pt([11, 13])
        km.add_data_pt([0, 0])
        km.add_data_pt([1, 0])
        km.add_data_pt([9.4, 10])
        km.add_data_pt([9.2, 11])
        km.add_data_pt([0, 0])
        self.assertEqual(km.get_data_point(0)[0], 1)
        self.assertEqual(km.get_data_point(0)[1], 2)
        # print data
        n = km.get_number_of_data_points()
        print "Number of points: ", n
        self.assertEqual(n,9)
        for i in range(0,n):
            print i
            km.get_data_point(i)
        # cluster data
        k = 2
        km.execute(k)
        # print centers one by one
        print "\nCenters:"
        # this should actually be equal to k
        nCenters = km.get_number_of_centers()
        print "Number of centers: ", nCenters
        self.assertEqual(nCenters, k)
        print range(1, nCenters)
        for i in range(0, nCenters):
            print "Center ", i, "   ", km.get_center(i)
        # make sure centers were found correctly
        c0 = km.get_center(0)
        c1 = km.get_center(1)
        if(c0[0] < 3.0):
            self.assertAlmostEqual(c0[0],0.4,1)
            self.assertAlmostEqual(c0[1],0.2,1)
        else:
            self.assertAlmostEqual(c0[0],9.9,1)
            self.assertAlmostEqual(c0[1],11.5,1)


if __name__ == '__main__':
    IMP.test.main()
