import IMP.display
import IMP.rmf
import RMF

# create a temporary file
tfn= IMP.create_temporary_file_name("rmf_geometry", "rmf")

# open the hdf5, clearing any existing contents
f= RMF.create_rmf_file(tfn)

# creating a box geometry
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10, 10, 10))
g= IMP.display.BoundingBoxGeometry(bb)

# add the geometry to the file
IMP.rmf.add_geometry(f, g)

# recreate the geometries from the file. The geometry will be the same
# but it will not be a IMP.display.BoundingBoxGeometry, it will be
# a set of cylinders instead.
gs= IMP.rmf.create_geometries(f, 0)
print gs[0].get_name()
print "Try running hdf5_display  on", tfn
