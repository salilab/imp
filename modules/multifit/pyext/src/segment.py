#!/usr/bin/env python

__doc__ = "Anchor graph segmentation of a density map."

import IMP.multifit
import IMP.em
from IMP import ArgumentParser


def parse_args():
    desc = """
Segments all voxels in the given density map, above the given threshold,
into the given number of clusters, and links between neighboring ones.

The cluster centers are written out into a single output PDB file, each
as a single CA atom.
"""
    p = ArgumentParser(description=desc)
    p.add_argument("--apix", type=float, default=None,
                   help="map spacing, in angstroms/pix (default: read "
                        "from MRC file)")
    p.add_argument("-x", "--x", type=float, default=None,
                   help="X origin of the density map")
    p.add_argument("-y", "--y", type=float, default=None,
                   help="Y origin of the density map")
    p.add_argument("-z", "--z", type=float, default=None,
                   help="Z origin of the density map")
    p.add_argument("--cmm", default="",
                   help="write results in CMM format")
    p.add_argument("--seg", default="",
                   help="write out each cluster as an MRC file called "
                        "<seg>_.mrc, and write load_segmentation.cmd file "
                        "to easily load all segments into Chimera")
    p.add_argument("--txt", default="",
                   help="write anchor points file in text format")
    p.add_argument("density", help="density map filename (in MRC format)")
    p.add_argument("num_cluster", type=int,
                   help="number of clusters")
    p.add_argument("threshold", type=float,
                   help="density threshold")
    p.add_argument("output", help="output PDB file name")
    return p.parse_args()


def main():
    args = parse_args()
    dmap = IMP.em.read_map(args.density, IMP.em.MRCReaderWriter())
    if args.apix is None:
        args.apix = dmap.get_spacing()
    else:
        dmap.update_voxel_size(args.apix)
    v = dmap.get_origin()
    if args.x is None:
        args.x = v[0]
    if args.y is None:
        args.y = v[1]
    if args.z is None:
        args.z = v[2]
    dmap.set_origin(args.x, args.y, args.z)
    dmap.set_was_used(True)
    IMP.multifit.get_segmentation(dmap, args.apix, args.threshold,
                                  args.num_cluster, args.output, args.cmm,
                                  args.seg, args.txt)

if __name__ == "__main__":
    main()
