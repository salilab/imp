## \example em2d/collision_cross_section.py
# Example of how to compute the collision cross section of a molecule.
#


from __future__ import print_function
import IMP
import IMP.em2d
import IMP.atom

"""

Example of how to compute the collision cross section of a molecule

"""


IMP.set_log_level(IMP.TERSE)
m = IMP.Model()

fn = IMP.em2d.get_example_path("1z5s.pdb")
prot = IMP.atom.read_pdb(fn, m, IMP.atom.ATOMPDBSelector())
IMP.atom.add_radii(prot)


projections = 20
resolution = 1.0
pixel_size = 1.5
img_size = 80
ccs = IMP.em2d.CollisionCrossSection(projections, resolution, pixel_size,
                                     img_size)
ccs.set_model_particles(IMP.atom.get_leaves(prot))
print("CCS", ccs.get_ccs(), "A**2")
