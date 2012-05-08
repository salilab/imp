import IMP
from IMP.kmeans import KMeansWrapper

def add_points(km):
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

km = KMeansWrapper()
add_points(km)
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
print "Number of centers (k): ", nCenters
for i in range(0, nCenters):
    print "Center ", i, "   ", km.getCenter(i)
