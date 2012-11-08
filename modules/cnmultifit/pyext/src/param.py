#!/usr/bin/python

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
    parser.add_option("-n", "--numsols", dest="numsols",default=10,
                      help="the number of solutions(fits) to report; "
                           "default 10")
    (options, args) = parser.parse_args()
    if len(args) != 9:
        parser.error("incorrect number of arguments")
    return options, args

def get_files_data(monomer_fn):
    monomer_ms_fn=monomer_fn+".ms"
    msg="# File Names:\n"
    msg+="monomer "+monomer_fn+"\n"
    msg+="monomer_ms "+monomer_ms_fn+"\n"
    msg+="log-file multifit.log\n"
    msg+="prot_lib "+ IMP.multifit.get_data_path("chem.lib") + "\n"
    return msg

def get_symmetry_data(cn_units,dn_units):
    msg="\n# Symmetry Parameters:\n";
    msg+="#  cn_symm_deg <Cyclic symmetry (trimer=3, tetramer=4, etc.)>\n";
    msg+="#  dn_symm_deg <Dihedral symmetry>\n";
    msg+="cn_symm_deg "+str(cn_units)+"\n";
    msg+="dn_symm_deg "+str(dn_units)+"\n";
    return msg

def get_scoring_data(liberal=True):
    msg="""
# Scoring Parameters:
#  scoreParams <small_interfaces_ratio> <max_penetration> <ns_thr>
#              <w1 w2 w3 w4 w5>
#  <small_interfaces_ratio> - the ratio of the low scoring transforms to
#                             be removed
#  <max_penetration> - maximal allowed penetration between molecule surfaces
#  <ns_thr> - normal score threshold
#  <w1 w2 w3 w4 w5> - scoring weights for ranges:
#              [-5.0,-3.6],[-3.6,-2.2],[-2.2,-1.0],[-1.0,1.0],[1.0-up]
"""
    if liberal:
        msg+="scoreParams 0.1 -10.0 0.5 -8 -4 0 1 0\n"
    else:
        msg+="scoreParams 0.1 -5.0 0.5 -8 -4 0 1 0\n"
    return msg

def get_density_data(density_map_fn,resolution,spacing,
                     threshold,pca_matching_thr,origin):
    msg="""
# Density Parameters:
#  <density> -the density map in MRC format
#  <density_resolution> -the resolution of the density map in A
#  <density_spacing> -the voxel spacing of the density in A
#  <density origin> - the origin of the map <x> <y> <z>
#  <density_threshold> - the threshold of the density map, used for
#                        PCA matching
#  <pca_matching_threshold> - corresponding principal components whose
#                             eigenvalues differ in less than
#                             pca_matching_threshold are considered to
#                             be a match
"""
    msg+="density "+density_map_fn +"\n"
    msg+="density_resolution "+str(resolution)+"\n"
    msg+="density_spacing "+str(spacing)+"\n"
    msg+="density_origin "+str(origin[0])+" " + str(origin[1]) +" " + str(origin[2])+"\n"
    msg+="density_threshold "+str(threshold)+"\n"
    msg+="pca_matching_threshold "+str(pca_matching_thr)+"\n"
    return msg

def get_log_data(intermediate_fn,output_fn,model_fn):
    msg  = "\n# Log Parameters:\n"
    msg += "output "+output_fn+"\n"
    msg += "model "+model_fn+"\n"
    if intermediate_fn != "":
        msg += "intermediate "+intermediate_fn+"\n"
    return msg

def get_clustering_data():
    msg="""
# Clustering Parameters:
#  clusterParams <axis_angle_thr DEGREES> <min_cluster_size>
#                <distance_between_centers_of_mass>
"""
    msg += "clusterParams 18 1 2.0\n";
    return msg;

def get_base_data():
    msg = "\n# Base Parameters:\n"
    msg += "#  baseParams <min_base_dist> <max_base_dist>\n"
    msg +="baseParams 5.0 50.0\n";
    return msg;

def get_grid_data():
    msg = "\n# Grid Parameters:\n"
    msg+="#  grid <grid_step> <max_distance> <vol_func_radius>\n"
    msg += "grid 0.5 6.0 6.0\n"
    return msg

def get_surface_data():
    msg="""
# Surface Parameters:
#  surfacePruneTheshold - threshold for surface pruning, i.e. no 2 points with
#                         distance below this value are left for matching
"""
    msg+="surfacePruneThreshold 1.5\n";
    return msg;

def get_fitting_data(num_sols):
    msg="\n# Fitting Parameters:\n";
    msg+="#  fitting - number of solutions to fit\n"
    msg+="fitting "+str(num_sols)+"\n";
    return msg;

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
    f.write(get_files_data(unit_pdb_fn))
    f.write(get_symmetry_data(cn_units,dn_units))
    f.write(get_scoring_data(liberal))
    f.write(get_density_data(density_map_fn,resolution,spacing,threshold,
                             pca_matching_thr,origin))
    f.write(get_log_data(intermediate_fn,output_fn,model_fn))
    f.write("\n\n#######   Advanced Parameters   #######\n\n")
    f.write(get_clustering_data())
    f.write(get_base_data())
    f.write(get_grid_data())
    f.write(get_surface_data())
    f.write(get_fitting_data(options.numsols))
    f.close()

if __name__ == "__main__":
    main()
