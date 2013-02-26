## \example em2d/collision_cross_section.py
## Example of how to compute the collision cross section of a molecule.
##


import IMP
import IMP.em2d as em2d
import IMP.atom as atom

"""

Example of how to compute the collision cross section of a molecule

"""


IMP.base.set_log_level(IMP.base.TERSE)
m = IMP.Model()

fn = em2d.get_example_path("1z5s.pdb")
prot =  atom.read_pdb(fn, m ,atom.ATOMPDBSelector())
atom.add_radii(prot)


projections = 20
resolution = 1.0
pixel_size = 1.5
img_size = 80
ccs = em2d.CollisionCrossSection(projections, resolution, pixel_size, img_size)
ccs.set_model_particles(IMP.atom.get_leaves(prot))
print "CCS",ccs.get_ccs(),"A**2"
