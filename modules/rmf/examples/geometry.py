## \example rmf/geometry.py
## This example shows writting one piece of geometry to an hdf5 and then reading it back.

import IMP.display
import IMP.rmf
import RMF

# create a temporary file
tfn= IMP.create_temporary_file_name("rmf_geometry", ".rmf")

# open the hdf5, clearing any existing contents
f= RMF.create_rmf_file(tfn)

# creating a box geometry
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10, 10, 10))
g= IMP.display.BoundingBoxGeometry(bb)

# add the geometry to the file
IMP.rmf.add_geometry(f, g)
IMP.rmf.save_frame(f, 0)

bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1,1,1),
                              IMP.algebra.Vector3D(10, 10, 10))
g.set_geometry(bb)
# save a second frame with the bounding box
IMP.rmf.save_frame(f, 1)


del f
f= RMF.open_rmf_file_read_only(tfn)
# recreate the geometries from the file. The geometry will be the same
# but it will not be a IMP.display.BoundingBoxGeometry, it will be
# a set of cylinders instead.
gs= IMP.rmf.create_geometries(f)
IMP.rmf.load_frame(f, 0)
print gs[0].get_name()
print "Try running rmf_display  on", tfn

# load another frame
IMP.rmf.load_frame(f, 1)
# cast it to a BoundingBoxGeometry and print out the geometry
print IMP.display.BoundingBoxGeometry.get_from(gs[0]).get_geometry()
