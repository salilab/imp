import IMP.em
import IMP.display

bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1,1,1),
                              IMP.algebra.Vector3D(11,11,11))
dm= IMP.em.create_density_map(bb, 2.5)
dmbb= IMP.em.get_bounding_box(dm)
print dmbb, bb

for i in range(0, dm.get_number_of_voxels()):
    dm.set_value(i,1)

nm=IMP.create_temporary_file_name("cube", ".mrc")
print nm
IMP.em.write_map(dm, nm)

nm=IMP.create_temporary_file_name("cube", ".py")
print nm
w= IMP.display.ChimeraWriter(nm)

for v in IMP.algebra.get_vertices(bb):
    g= IMP.display.SphereGeometry(IMP.algebra.Sphere3D(v, 1))
    w.add_geometry(g)
