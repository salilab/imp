## \example em/cube.py
# The example creates a simple mrc file that is filled uniformly with 1s.
# In addition, an RMF file is written with a marker at each corner of the
# density. When both files are opened (e.g. in Chimera), the density should
# be centered among the markers. This can be used for testing for
# registration errors when reading and writing density maps.

import IMP.em
import IMP.display
import IMP.rmf
import RMF
import sys

IMP.setup_from_argv(sys.argv, "cube")

bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(1, 1, 1),
                               IMP.algebra.Vector3D(11, 11, 11))
dm = IMP.em.create_density_map(bb, 2.5)
dmbb = IMP.em.get_bounding_box(dm)
print(dmbb, bb)

for i in range(0, dm.get_number_of_voxels()):
    dm.set_value(i, 1)

nm = IMP.create_temporary_file_name("cube", ".mrc")
print(nm)
IMP.em.write_map(dm, nm)

nm = IMP.create_temporary_file_name("cube", ".rmf")
print(nm)
w = RMF.create_rmf_file(nm)

for v in IMP.algebra.get_vertices(bb):
    g = IMP.display.SphereGeometry(IMP.algebra.Sphere3D(v, 1))
    IMP.rmf.add_geometry(w, g)
IMP.rmf.save_frame(w, "zero")
