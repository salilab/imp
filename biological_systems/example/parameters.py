import IMP.algebra

# the spring constant to use, it doesn't really matter
k=100
# the target resolution for the representation
resolution=100
# the box to perform everything in
bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                             IMP.algebra.Vector3D(300, 300, 300))
# the number of clusters to build when analyzing
number_of_clusters=10
