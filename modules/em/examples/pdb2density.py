## \example em/pdb2density.py
## A simple example showing how to simulate density from a protein.
## IMP uses a Gaussian smoothing kernel. see SampledDensityMap::resample for documentation.
##

import IMP.em
import IMP.core
import IMP.atom
m= IMP.Model()
#read protein
sel=IMP.atom.NonWaterPDBSelector()
mh=IMP.atom.read_pdb(IMP.em.get_example_path("input.pdb"),m,sel)
#add radius info to each atom, otherwise the resampling would fail.
IMP.atom.add_radii(mh)
ps= IMP.core.get_leaves(mh)
#decide on resolution and spacing you would like to simulate to
resolution=10.
apix=1.5
dmap=IMP.em.particles2density(ps,resolution,apix)
#write out the map in the favorite format (xplor, mrc, em and spider are supported)
IMP.em.write_map(dmap,"example.mrc",IMP.em.MRCReaderWriter())
