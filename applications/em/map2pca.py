from optparse import OptionParser
import IMP.em
def main():
    IMP.set_log_level(IMP.SILENT)
    usage = "usage: %prog [options] <in_density> <density threshold> <out_pca.cmm> \n Description: Calculates the map principle components and writes them in bild format. The 3D points participating in the PCA calculation are the centers of voxels with density above the input threshold\n"
    parser = OptionParser(usage)
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
    dens_vecs = IMP.em.density2vectors(dmap,threshold);
    dens_pca = IMP.algebra.get_principal_components(dens_vecs);
    f=open(out_pca_fn,"w")
    dens_pca.show(f)
    f.close()

if __name__ == "__main__":
    main()
