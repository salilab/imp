## \example display/show_particles_as_spheres.py
# Write two colored set of random IMP::core::XYZR particles to a Pymol
# input file.

import IMP
import IMP.algebra
import IMP.container
import IMP.core
import IMP.display


# create two lists of random particles for display
m = IMP.Model()
n = 10
radius = 1.0
bounding_box_size = 10.0
xyzrs0 = IMP.core.create_xyzr_particles(m, 10, radius, bounding_box_size)
xyzrs1 = IMP.core.create_xyzr_particles(m, 10, radius, bounding_box_size)
xyzrs0_container = IMP.container.ListSingletonContainer(m, xyzrs0)
xyzrs1_container = IMP.container.ListSingletonContainer(m, xyzrs1)

# create a writer that generates Pymol Python scripts for visualizing
# the particle lists
w = IMP.display.PymolWriter("out.pym")
# write first list of particles
g0 = IMP.core.XYZRsGeometry(xyzrs0_container)
g0.set_name("my particles")
g0.set_color(IMP.display.Color(1, 0, 0))
w.add_geometry(g0)
# write second list of particles
g1 = IMP.core.XYZRsGeometry(xyzrs1_container)
g1.set_name("my other particles")
g1.set_color(IMP.display.Color(0, 1, 0))
w.add_geometry(g1)
# make sure that the file is flushed
del w
