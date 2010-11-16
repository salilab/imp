from optparse import OptionParser
import IMP.em
def main():
    IMP.set_log_level(IMP.SILENT)
    usage = "usage: %prog [options] <em map> <resample level 2/3/4...> <output: resample_density.mrc>  \n Description: Resample a density map."
    parser = OptionParser(usage)
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
