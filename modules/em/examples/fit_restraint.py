## \example em/fit_restraint.py
## A simple example showing how to set up a fit restraint. The number of spheres and resolution are randomly chosen and so should not be considered significant.

import IMP.em
import IMP.core
import IMP.atom
IMP.base.set_log_level(IMP.base.SILENT)
m= IMP.Model()
#1. setup the input protein
##1.1 select a selector.
sel=IMP.atom.NonWaterPDBSelector()
##1.2 read the protein
mh=IMP.atom.read_pdb(IMP.em.get_example_path("input.pdb"),m,sel)
ps=IMP.core.get_leaves(mh)
IMP.atom.add_radii(mh)
#2. read the density map
resolution=8.
voxel_size=1.5
dmap=IMP.em.read_map(IMP.em.get_example_path("input.mrc"),IMP.em.MRCReaderWriter())
dmap.get_header_writable().set_resolution(resolution)
#3. calculate the cross correlation between the density and the map
print "The cross-correlation score is:",1.-IMP.em.compute_fitting_score(ps,dmap)
#4. add a fitting restraint
r= IMP.em.FitRestraint(ps, dmap)
m.add_restraint(r)
print "The fit of the particles in the density is:",r.evaluate(False)
