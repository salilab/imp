from optparse import OptionParser
import IMP.em
def main():
    IMP.set_log_level(IMP.SILENT)
    usage = "usage: %prog [options] <in_protein> <out_pca.cmm> \n Description: Calculates the protein principle components and writes them in cmm format. The 3D points participating in the PCA calculation are the centers of voxels with density above the input threshold\n"
    parser = OptionParser(usage)
    (options, args) = parser.parse_args()
    if len(args) != 2:
        parser.error("incorrect number of arguments")
    in_prot_fn=args[0]
    out_pca_fn=args[1]
    mdl=IMP.Model()
    mol=IMP.atom.read_pdb(in_prot_fn,mdl)
    vecs=[]
    for xyz in IMP.core.XYZs(IMP.core.get_leaves(mol)):
        vecs.append(xyz.get_coordinates())
    pca = IMP.algebra.get_principal_components(vecs);
    f=open(out_pca_fn,"w")
    pca.show(f)
    f.close()

if __name__ == "__main__":
    main()
