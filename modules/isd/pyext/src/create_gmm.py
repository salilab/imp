from __future__ import print_function
import IMP
import IMP.em
import IMP.isd
import IMP.isd.gmm_tools
from IMP import ArgumentParser

import sys,os

def parse_args():
    desc = """
    Create a GMM from either density file (.mrc), a pdb file (.pdb)
    Will detect input format from extension.
    Outputs as text and optionally as a density map
    see help(-h)
"""
    p  = ArgumentParser(description=desc)

    p.add_argument("-t","--covar_type",dest="covar_type",default='full',
                      choices=['spherical', 'tied', 'diag', 'full'],
                      help="covariance type for the GMM")
    p.add_argument("-m","--out_map",dest="out_map",default='',
                      help="write out the gmm to an mrc file")
    p.add_argument("-a","--apix",dest="apix",default=1.0,type=float,
                      help="if you don't provide a map, set the voxel_size here (for sampling)")
    p.add_argument("-n","--num_samples",dest="num_samples",default=1000000,type=int,
                      help="num samples to draw from the density map")
    p.add_argument("-i","--num_iter",dest="num_iter",default=100,type=int,
                      help="num iterations of GMM")
    p.add_argument("-s","--threshold",dest="threshold",default=0.0,type=float,
                      help="threshold for the map before sampling")

    p.add_argument("-f","--force_radii",dest="force_radii",default=-1.0,
                      type=float,
                      help="force radii to be this value (spherical) -1 means deactivated ")
    p.add_argument("-w","--force_weight",dest="force_weight",default=-1.0,
                      type=float,
                      help="force weight to be this value (spherical) -1 means deactivated ")
    p.add_argument("-e", "--force_weight_frac", dest="force_weight_frac",
                   action="store_true", default=False,
                   help="force weight to be 1.0/(num centers). "
                        "Takes precedence over -w")
    p.add_argument("-d","--use_dirichlet",dest="use_dirichlet",default=False,
                      action="store_true",
                      help="use dirichlet process for fit")

    p.add_argument("-k","--multiply_by_mass",dest="multiply_by_mass",default=False,
                      action="store_true",
                      help="if set, will multiply all weights by the total mass of the particles (PDB ONLY)")
    p.add_argument("-x","--chain",dest="chain",default=None,
                      help="If you passed a PDB file, read this chain")

    p.add_argument("-z","--use_cpp",dest="use_cpp",default=False,
                      action="store_true",
                      help="EXPERIMENTAL. Uses the IMP GMM code. Requires isd_emxl")
    p.add_argument("data_file", help="data file name")
    p.add_argument("n_centers", type=int, help="number of centers")
    p.add_argument("out_file", help="output file name")
    return p.parse_args()

def run(args):
    data_fn = args.data_file
    ncenters = args.n_centers
    out_txt_fn = args.out_file
    mdl = IMP.Model()

    if not os.path.isfile(data_fn):
        raise Exception("The data file you entered: "+data_fn+" does not exist!")

    ### get points for fitting the GMM
    ext = data_fn.split('.')[-1]
    mass_multiplier = 1.0
    if ext=='pdb':
        mh = IMP.atom.read_pdb(data_fn,mdl,IMP.atom.NonWaterNonHydrogenPDBSelector())
        if args.chain:
            mps = IMP.atom.Selection(mh,chain=args.chain).get_selected_particles()
        else:
            mps = IMP.core.get_leaves(mh)

        if args.multiply_by_mass:
            mass_multiplier=sum(IMP.atom.Mass(p).get_mass() for p in mps)

        pts = [IMP.core.XYZ(p).get_coordinates() for p in mps]
        bbox = None
    elif ext=='mrc':
        dmap = IMP.em.read_map(data_fn,IMP.em.MRCReaderWriter())
        bbox = IMP.em.get_bounding_box(dmap)
        dmap.set_was_used(True)
        print('sampling points')
        pts = IMP.isd.sample_points_from_density(dmap,args.num_samples,args.threshold)
    else:
        raise ValueError("data_fn extension must be pdb or mrc")

    ### Do fitting to points
    if not args.use_cpp:
        density_ps = []
        print('fitting gmm')
        #IMP.isd_emxl.gmm_tools.draw_points(pts,'test_points.bild')

        if args.force_weight_frac:
            force_weight = 1.0/ncenters
        else:
            force_weight=args.force_weight
        if force_weight != -1:
            print('weight forced to',force_weight)
        if not args.use_dirichlet:
            gmm = IMP.isd.gmm_tools.fit_gmm_to_points(pts,ncenters,mdl,density_ps,
                                                      args.num_iter,args.covar_type,
                                                      force_radii=args.force_radii,
                                                      force_weight=args.force_weight,
                                                      mass_multiplier=mass_multiplier)
        else:
            gmm = IMP.isd.gmm_tools.fit_dirichlet_gmm_to_points(pts,ncenters,mdl,density_ps,
                                                                args.num_iter,args.covar_type,
                                                                mass_multiplier=mass_multiplier)

    else:
        try:
            import isd_emxl
        except ImportError:
            print("This option is experimental, only works if you have isd_emxl")
        gmm_threshold = 0.01
        density_ps = IMP.isd_emxl.fit_gaussians_to_density(mdl,dmap,args.num_samples,
                                                         ncenters,args.num_iter,
                                                         args.threshold,
                                                         gmm_threshold)

    ### Write to files
    comments = ['Created by create_gmm.py, IMP.isd version %s'
                % IMP.isd.get_module_version()]
    comments.append('data_fn: ' + IMP.get_relative_path(out_txt_fn, data_fn))
    comments.append('ncenters: %d' % ncenters)
    for key in ('covar_type', 'apix', 'num_samples', 'num_iter',
                'threshold', 'force_radii', 'force_weight',
                'force_weight_frac', 'use_dirichlet', 'multiply_by_mass',
                'chain'):
        comments.append('%s: %s' % (key, repr(getattr(args, key))))
    IMP.isd.gmm_tools.write_gmm_to_text(density_ps, out_txt_fn, comments)
    if args.out_map != '':
        IMP.isd.gmm_tools.write_gmm_to_map(density_ps, args.out_map,
                                           args.apix, bbox)

def main():
    args = parse_args()
    run(args)

if __name__=="__main__":
    main()
