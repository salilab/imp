from __future__ import print_function
import IMP
import IMP.em
import IMP.isd_emxl
import IMP.isd_emxl.gmm_tools
import numpy as np
from optparse import OptionParser
import sys,os
def parse_args():
    usage = """%prog [options] <data_fn> <n_components> <out_txt_fn>
    Create a GMM from either density file (.mrc), a pdb file (.pdb)
    Will detect input format from extension.
    Outputs as text and optionally as a density map
    see help(-h)
"""
    parser = OptionParser(usage)
    parser.add_option("-t","--covar_type",dest="covar_type",default='full',
                      choices=['spherical', 'tied', 'diag', 'full'],
                      help="covariance type for the GMM")
    parser.add_option("-m","--out_map",dest="out_map",default='',
                      help="write out the gmm to an mrc file")
    parser.add_option("-a","--apix",dest="apix",default=1.0,type='float',
                      help="if you don't provide a map, set the voxel_size here (for sampling)")
    parser.add_option("-r","--resolution",dest="resolution",default=1,type='float',
                      help="if you don't provide a map, set the resolution here (for sampling)")
    parser.add_option("-n","--num_samples",dest="num_samples",default=1000000,type='int',
                      help="num samples to draw from the density map")
    parser.add_option("-i","--num_iter",dest="num_iter",default=100,type='int',
                      help="num iterations of GMM")
    parser.add_option("-s","--threshold",dest="threshold",default=0.0,type='float',
                      help="threshold for the map before sampling")

    parser.add_option("-c","--input_anchors_fn",dest="input_anchors_fn",default='',
                      help="get initial centers from anchors file. ")
    parser.add_option("-f","--force_radii",dest="force_radii",default=-1.0,
                      type='float',
                      help="force radii to be this value (spherical) -1 means deactivated ")
    parser.add_option("-w","--force_weight",dest="force_weight",default=-1.0,
                      type='float',
                      help="force weight to be this value (spherical) -1 means deactivated ")
    parser.add_option("-e","--force_weight_frac",dest="force_weight_frac",action="store_true",default=False,
                      help="force weight to be 1.0/(num anchors). takes precedence over -w ")
    parser.add_option("-o","--out_anchors_txt",dest="out_anchors_txt",default='',
                      help="write final GMM as anchor points (txt)")
    parser.add_option("-q","--out_anchors_cmm",dest="out_anchors_cmm",default='',
                      help="write final GMM as anchor points (cmm)")
    parser.add_option("-d","--use_dirichlet",dest="use_dirichlet",default=False,
                      action="store_true",
                      help="use dirichlet process for fit")

    parser.add_option("-k","--multiply_by_mass",dest="multiply_by_mass",default=False,
                      action="store_true",
                      help="if set, will multiply all weights by the total mass of the particles (PDB ONLY)")

    parser.add_option("-z","--use_cpp",dest="use_cpp",default=False,
                      action="store_true",
                      help="use the IMP GMM code")


    (options, args) = parser.parse_args()
    if len(args) != 3:
        parser.error("incorrect number of arguments")
    return options,args

def run():
    options,args=parse_args()
    data_fn,ncenters,out_txt_fn=args
    ncenters=int(ncenters)
    mdl=IMP.Model()

    if not os.path.isfile(data_fn):
        raise Exception("The data file you entered: "+data_fn+" does not exist!")

    ### get points for fitting the GMM
    ext=data_fn.split('.')[-1]
    mass_multiplier=1.0
    if ext=='pdb':
        mh=IMP.atom.read_pdb(data_fn,mdl,IMP.atom.NonWaterNonHydrogenPDBSelector())
        mps=IMP.core.get_leaves(mh)
        dmap=IMP.em.SampledDensityMap(mps,
                                      options.resolution,
                                      options.apix)
        dmap.calcRMS()
        if options.multiply_by_mass:
            mass_multiplier=sum(IMP.atom.Mass(p).get_mass() for p in mps)
    elif ext=='mrc':
        dmap=IMP.em.read_map(data_fn,IMP.em.MRCReaderWriter())
        #dmap.set_origin(-56/options.apix,-56/options.apix,-56/options.apix)
    else:
        print('ERROR: data_fn extension must be pdb, mrc, or npy')
        sys.exit()

    if not options.use_cpp:
        print('sampling points')
        pts=IMP.isd_emxl.sample_points_from_density(dmap,options.num_samples,options.threshold)
        density_ps=[]
        print('fitting gmm')
        #IMP.isd_emxl.gmm_tools.draw_points(pts,'test_points.bild')

        if options.force_weight_frac:
            force_weight=1.0/ncenters
        else:
            force_weight=options.force_weight
        if force_weight!=-1:
            print('weight forced to',force_weight)
        if not options.use_dirichlet:
            gmm=IMP.isd_emxl.gmm_tools.fit_gmm_to_points(pts,ncenters,mdl,density_ps,
                                                         options.num_iter,options.covar_type,
                                                         force_radii=options.force_radii,
                                                         force_weight=options.force_weight,
                                                         mass_multiplier=mass_multiplier)
        else:
            gmm=IMP.isd_emxl.gmm_tools.fit_dirichlet_gmm_to_points(pts,ncenters,mdl,density_ps,
                                                         options.num_iter,options.covar_type,
                                                         mass_multiplier=mass_multiplier)

    else:
        gmm_threshold=0.01
        density_ps=IMP.isd_emxl.fit_gaussians_to_density(mdl,dmap,options.num_samples,
                                                         ncenters,options.num_iter,
                                                         options.threshold,
                                                         gmm_threshold)

    IMP.isd_emxl.gmm_tools.write_gmm_to_text(density_ps,out_txt_fn)
    if options.out_map!='':
        IMP.isd_emxl.gmm_tools.write_gmm_to_map(density_ps,options.out_map,
                                                options.apix,
                                                IMP.em.get_bounding_box(dmap))

    if options.out_anchors_txt!='':
        IMP.isd_emxl.gmm_tools.write_gmm_to_anchors(density_ps,options.out_anchors_txt,
                                                options.out_anchors_cmm)



if __name__=="__main__":
    run()
