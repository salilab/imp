## \example kmeans/kmeans_example.py
## Create a set and points and cluster it, printing the resulting cluster centers

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
# cluster data
k = 2
km.execute(k)
# print centers one by one
print "\nCenters:"
nCenters = km.get_number_of_centers() # this should actually be equal to k
print "Number of centers (k): ", nCenters
for i in range(0, nCenters):
    print "Center ", i, "   ", km.get_center(i)

# print data point assignments
n = km.get_number_of_data_points()
assignments = km.get_assignments()
sqrdist = km.get_squared_distance_to_centers()
print "Number of points: ", n
for i in range(0,n):
    print "Point ", i , ": "
    print km.get_data_point(i)
    print "Cluster center: ", assignments[i], \
        " sqrt-dist: ", sqrdist[i]
