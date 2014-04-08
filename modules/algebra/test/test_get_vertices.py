import IMP.algebra

bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                               IMP.algebra.Vector3D(10, 10, 10))
# swig has had trouble with this
IMP.algebra.get_vertices(bb)
