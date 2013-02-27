#!/usr/bin/env python

import IMP.em

def main():
    IMP.base.set_log_level(IMP.base.SILENT)
    usage = """%prog [options] <em map> <resample level 2/3/4...>
             <output: resample_density.mrc>

Resample a density map."""
    parser = IMP.OptionParser(usage=usage, imp_module=IMP.em)
    (options, args) = parser.parse_args()
    if len(args) != 3:
        parser.error("incorrect number of arguments")
    in_map_fn=args[0]
    resample_level=int(args[1])
    output_map_fn=args[2]
    #read the protein
    dmap=IMP.em.read_map(in_map_fn)
    r_dmap = IMP.em.get_resampled(dmap,resample_level)
    IMP.em.write_map(r_dmap,output_map_fn,IMP.em.MRCReaderWriter())

if __name__ == "__main__":
    main()
