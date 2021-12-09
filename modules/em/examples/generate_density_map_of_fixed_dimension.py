## \example em/generate_density_map_of_fixed_dimension.py
# Shows how to generate a density map of fixed dimension and how to sample
# particles within this density map.

from __future__ import print_function
import IMP
import IMP.atom
import IMP.em
import sys

IMP.setup_from_argv(sys.argv, "generate a density map of fixed dimension")

m = IMP.Model()
# 1. read a protein and get its bounding box dimension
sel = IMP.atom.CAlphaPDBSelector()
# read protein
mh = IMP.atom.read_pdb(IMP.em.get_example_path("input.pdb"), m, sel)
apix = 1.
resolution = 6.
# compute bbox, and map size in voxels
bbox = IMP.atom.get_bounding_box(mh)

# create a density header of the requested size
dheader = IMP.em.create_density_header(bbox, apix)
dheader.set_resolution(resolution)
dmap = IMP.em.SampledDensityMap(dheader)
ps = IMP.core.get_leaves(mh)
dmap.set_particles(ps)
# dmap.get_header_writable().set_number_of_voxels(nx,ny,nz)
dmap.resample()
# computes statistic stuff about the map and insert it in the
# header
dmap.calcRMS()
print(dmap.get_header().show(), "\n")
IMP.em.write_map(dmap, "map.mrc", IMP.em.MRCReaderWriter())
