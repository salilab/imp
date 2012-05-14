import IMP
from IMP.kmeans import KMeans

def add_points(km):
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

km = KMeans()
add_points(km)
# print data
n = km.get_n_data_points()
print "Number of points: ", n
for i in range(0,n):
    print i
    km.get_data_point(i)
# cluster data
k = 2
km.execute(k)
# print centers one by one
print "\nCenters:"
nCenters = km.get_n_centers() # this should actually be equal to k
print "Number of centers (k): ", nCenters
for i in range(0, nCenters):
    print "Center ", i, "   ", km.get_center(i)
