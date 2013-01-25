## \example algebra/grid_space.py
## This example shows how to use the grid support in IMP.algebra to discretize a set of continuous points. In this case the points are simply randomly drawn from the surface of a sphere, but they could be taken from something more interesting.

import IMP.algebra

# create a unit grid with its origin at 0,0,0
g= IMP.algebra.SparseUnboundedIntGrid3D(1, IMP.algebra.Vector3D(0,0,0))

s= IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1,1,1), 6)
count=0
for i in range(0,100):
    p= IMP.algebra.get_random_vector_on(s)
    ei= g.get_extended_index(p)
    if g.get_has_index(ei):
        print "hit"
    else:
        g.add_voxel(ei, count)
        count=count+1
print "There are", len(g.get_all_indexes()), "distinct values", count
