import IMP.algebra

# create a unit grid with its origin at 0,0,0
g= IMP.algebra.SparseUnboundedIntGrid3D(1, IMP.algebra.Vector3D(0,0,0))

s= IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1,1,1), 6)
count=0
for i in range(0,100):
    p= IMP.algebra.get_random_vector_on(s)
    if g.get_has_index(p):
        print "hit"
    else:
        g.add_voxel(p, count)
        count=count+1
print "There are", len(g.get_all_indexes()), "distinct values", count
