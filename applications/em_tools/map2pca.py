#!/usr/bin/env python

import IMP.em

def main():
    IMP.base.set_log_level(IMP.base.SILENT)
    usage = """%prog [options] <in_density> <density threshold> <out_pca.cmm>

Calculates the map principal components and writes them in cmm format.
The 3D points participating in the PCA calculation are the centers of voxels
with density above the input threshold."""
    parser = IMP.OptionParser(usage=usage, imp_module=IMP.em)
    parser.add_option("-p", "--apix", dest="apix",
                      help="voxel size")
    (options, args) = parser.parse_args()
    if len(args) != 3:
        parser.error("incorrect number of arguments")
    in_map_fn=args[0]
    threshold = float(args[1])
    out_pca_fn=args[2]
    dmap=IMP.em.read_map(in_map_fn)
    if options.apix:
        dmap.update_voxel_size(float(options.apix))
    dens_vecs = IMP.em.density2vectors(dmap,threshold)
    dens_pca = IMP.algebra.get_principal_components(dens_vecs)
    f=open(out_pca_fn,"w")
    IMP.em.write_pca_cmm(dens_pca, f)
    f.close()

if __name__ == "__main__":
    main()
