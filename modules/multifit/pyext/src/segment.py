#!/usr/bin/env python

__doc__ = "Anchor graph segmentation of a density map."

import IMP.multifit
import IMP.em
from IMP import OptionParser

def parse_args():
    usage = """%prog [options] <density.mrc> <number of clusters>
              <density threshold> <output.pdb>

Segments all voxels in the given density map, above the given threshold,
into the given number of clusters, and links between neighboring ones.

The cluster centers are written out into a single output PDB file, each
as a single CA atom.
"""
    parser = OptionParser(usage)
    parser.add_option("--apix", type="float", default=None,
                      help="map spacing, in angstroms/pix (default: read "
                           "from MRC file)")
    parser.add_option("-x", "--x", type="float", default=None,
                      help="X origin of the density map")
    parser.add_option("-y", "--y", type="float", default=None,
                      help="Y origin of the density map")
    parser.add_option("-z", "--z", type="float", default=None,
                      help="Z origin of the density map")
    parser.add_option("--cmm", type="str", default="",
                      help="write results in CMM format")
    parser.add_option("--seg", type="str", default="",
                      help="write out each cluster as an MRC file called "
                           "<seg>_.mrc, and write load_segmentation.cmd file "
                           "to easily load all segments into Chimera")
    parser.add_option("--txt", type="str", default="",
                      help="write anchor points file in text format")
    options, args = parser.parse_args()

    if len(args) != 4:
        parser.error("incorrect number of arguments")
    return options, args


def main():
    opts, args = parse_args()
    asmb_fn, num_cluster, threshold, output = args
    num_cluster = int(num_cluster)
    threshold = float(threshold)
    dmap = IMP.em.read_map(asmb_fn, IMP.em.MRCReaderWriter())
    if opts.apix is None:
        opts.apix = dmap.get_spacing()
    else:
        dmap.update_voxel_size(opts.apix)
    v = dmap.get_origin()
    if opts.x is None:
        opts.x = v[0]
    if opts.y is None:
        opts.y = v[1]
    if opts.z is None:
        opts.z = v[2]
    dmap.set_origin(opts.x, opts.y, opts.z)
    dmap.set_was_used(True)
    IMP.multifit.get_segmentation(dmap, opts.apix, threshold, num_cluster,
                                  output, opts.cmm, opts.seg, opts.txt)

if __name__=="__main__":
    main()
