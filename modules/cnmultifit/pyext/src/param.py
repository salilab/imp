#!/usr/bin/env python

__doc__ = "Generate a suitable parameter file for build_models."

import IMP.multifit
from optparse import OptionParser

def usage():
    usage = """%prog [options] <cyclic symmetry degree>
       <monomer PDB file> <density map> <resolution> <spacing>
       <density threshold> <origin X> <origin Y> <origin Z>

A script that builds the parameters file for symmetric MultiFit.

Notice: If you have negative numbers as input, add -- as the first parameter,
so that the numbers are not treated as options."""

    parser = OptionParser(usage)
    parser.add_option("-o", "--out", dest="out",default="multifit.output",
                      metavar="FILE",
                      help="the name of the MultiFit output file. The default "
                           "filename is multifit.output")
    parser.add_option("-i", "--med", dest="med", metavar="FILE", default="",
                      help="Print intermediate results to the named file.")
    parser.add_option("-p", "--params", dest="params",default="multifit.param",
                      help="the name of the MultiFit parameters file. The "
                           "default filename is multifit.params")
    parser.add_option("-m", "--model", dest="model",default="asmb.model",
                      help="the base filename of the solutions output by "
                           "MultiFit (.X.pdb, where X is the solution number, "
                           "is suffixed to create each output file name). "
                           "The default filename is asmb.model")
    parser.add_option("-n", "--numsols", dest="numsols",default=10, type="int",
                      help="the number of solutions(fits) to report; "
                           "default 10")
    (options, args) = parser.parse_args()
    if len(args) != 9:
        parser.error("incorrect number of arguments")
    return options, args

def get_files_data(monomer_fn, intermediate_fn, output_fn, model_fn):
    monomer_ms_fn=monomer_fn+".ms"
    msg="[files]\n"
    msg+="monomer = "+monomer_fn+"\n"
    msg+="surface = "+monomer_ms_fn+"\n"
    msg+="prot_lib = "+ IMP.multifit.get_data_path("chem.lib") + "\n"
    msg += "output = "+output_fn+"\n"
    msg += "model = "+model_fn+"\n"
    if intermediate_fn != "":
        msg += "intermediate = "+intermediate_fn+"\n"
    return msg

    return msg

def get_symmetry_data(cn_units,dn_units):
    msg="\n[symmetry]\n"
    msg+="; Cyclic symmetry (trimer=3, tetramer=4, etc.)\n"
    msg+="cn = "+str(cn_units)+"\n"
    msg+="; Dihedral symmetry\n"
    msg+="dn = "+str(dn_units)+"\n"
    return msg

def get_scoring_data(liberal=True):
    if liberal:
        penetration = "-10.0"
    else:
        penetration = "-5.0"
    msg="""
[scoring]
; the ratio of the low scoring transforms to be removed
small_interface_ratio = 0.1
; maximal allowed penetration between molecule surfaces
max_penetration = %s
; normal score threshold
threshold = 0.5
; scoring weights for ranges [-5.0,-3.6], [-3.6,-2.2], [-2.2,-1.0],
;                            [-1.0,1.0], [1.0-up] respectively
weight1 = -8
weight2 = -4
weight3 = 0
weight4 = 1
weight5 = 0
""" % penetration
    return msg

def get_density_data(density_map_fn,resolution,spacing,
                     threshold,pca_matching_thr,origin):
    msg="""
[density]
; the density map in MRC format
map = %s
; the resolution of the density map in A
resolution = %s
; the voxel spacing of the density in A
spacing = %s
; the origin of the map
origin_x = %s
origin_y = %s
origin_z = %s
; the threshold of the density map, used for PCA matching
threshold = %s
; corresponding principal components whose eigenvalues differ in less than
; pca_matching_threshold are considered to be a match
pca_matching_threshold = %s
""" % (density_map_fn, resolution, spacing, origin[0], origin[1], origin[2],
       threshold, pca_matching_thr)
    return msg

def get_clustering_data():
    msg="""
[clustering]
; angle in degrees
axis_angle_threshold = 18
min_size = 1
; distance between centers of mass
distance = 2.0
"""
    return msg

def get_base_data():
    msg = """
[base]
min_distance = 5.0
max_distance = 50.0
"""
    return msg

def get_grid_data():
    msg = """
[grid]
step = 0.5
max_distance = 6.0
volume_radius = 6.0
"""
    return msg

def get_surface_data():
    msg="""
[surface]
; threshold for surface pruning, i.e. no 2 points with distance below this
; value are left for matching
threshold = 1.5
"""
    return msg

def get_fitting_data(num_sols):
    msg="""
[fitting]
; number of solutions to fit
solutions = %d
""" % num_sols
    return msg

def main():
    options,args = usage()
    cn_units = int(args[0])
    dn_units = 1

    liberal=False #TODO - make a parameter
    unit_pdb_fn = args[1]
    density_map_fn = args[2]
    resolution=float(args[3])
    spacing=args[4]
    threshold=args[5]
    origin=args[6:]
    if resolution>15:
        pca_matching_thr=15
    else:
        pca_matching_thr=resolution*0.75
    params_fn=options.params
    intermediate_fn=options.med
    log_fn="multifit.log"
    output_fn=options.out
    model_fn=options.model
    f=open(params_fn,"w")
    f.write(get_files_data(unit_pdb_fn, intermediate_fn, output_fn, model_fn))
    f.write(get_symmetry_data(cn_units,dn_units))
    f.write(get_scoring_data(liberal))
    f.write(get_density_data(density_map_fn,resolution,spacing,threshold,
                             pca_matching_thr,origin))
    f.write("\n\n; #######   Advanced Parameters   #######\n")
    f.write(get_clustering_data())
    f.write(get_base_data())
    f.write(get_grid_data())
    f.write(get_surface_data())
    f.write(get_fitting_data(options.numsols))
    f.close()

if __name__ == "__main__":
    main()
