from optparse import OptionParser
import IMP.em
def main():
    IMP.set_log_level(IMP.SILENT)
    usage = "usage: %prog [options] <complex.pdb> <output: density.mrc> <resolution> <a/pix> \n Description: Samples a protein into a simulated 3D density map."
    parser = OptionParser(usage)
    (options, args) = parser.parse_args()
    if len(args) != 4:
        parser.error("incorrect number of arguments")
    pdb_fn=args[0]
    mrc_fn=args[1]
    resolution=float(args[2])
    apix=float(args[3])
    #read the protein
    mdl=IMP.Model()
    mh=IMP.atom.read_pdb(pdb_fn,mdl,IMP.atom.NonWaterNonHydrogenPDBSelector())
    IMP.atom.add_radii(mh)
    #sample the density
    dmap = IMP.em.particles2density(IMP.core.get_leaves(mh),resolution,apix)
    IMP.em.write_map(dmap,mrc_fn,IMP.em.MRCReaderWriter())

if __name__ == "__main__":
    main()
