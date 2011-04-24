import IMP.display
import IMP.rmf
tfn= IMP.create_temporary_file_name("hdf5_geometry", "hdf5")
f= IMP.rmf.RootHandle(tfn, True)
bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10, 10, 10))
g= IMP.display.BoundingBoxGeometry(bb)
IMP.rmf.add_geometry(f, g)
gs= IMP.rmf.create_geometries(f, 0)
print gs[0].get_name()
print "Try running hdf5_display  on", tfn
