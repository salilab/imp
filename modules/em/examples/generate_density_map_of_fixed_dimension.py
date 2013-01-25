## \example em/generate_density_map_of_fixed_dimension.py
## Shows how to generate a density map of fixed dimension and how to sample particles within this density map.

import IMP
import IMP.atom
import IMP.em
m= IMP.Model()
#1. read a protein and get its bounding box dimension
sel=IMP.atom.CAlphaPDBSelector()
# read protein
mh=IMP.atom.read_pdb(IMP.em.get_example_path("input.pdb"),m,sel)
apix=1.
resolution=6.
# compute bbox, and map size in voxels
bbox            = IMP.atom.get_bounding_box(mh)
#diag            = bbox.get_corner(0) - bbox.get_corner(1)
#nx              = int(bboxCoverage * diag[0] / apix)
#ny              = int(bboxCoverage * diag[1] / apix)
#nz              = int(bboxCoverage * diag[2] / apix)

#create a density header of the requested size
dheader = IMP.em.create_density_header(bbox,apix)
dheader.set_resolution(resolution)
dmap = IMP.em.SampledDensityMap(dheader)
ps = IMP.core.get_leaves(mh)
dmap.set_particles(ps)
#dmap.get_header_writable().set_number_of_voxels(nx,ny,nz)
dmap.resample()
dmap.calcRMS()      # computes statistic stuff about the map and insert it in the header
print dmap.get_header().show(),"\n"
IMP.em.write_map(dmap,"map.mrc",IMP.em.MRCReaderWriter())
