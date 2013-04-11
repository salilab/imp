#!/usr/bin/env python

__doc__ = "Refine fitting subunits into a density map with FFT."

import math
import IMP.multifit
import IMP.atom
import IMP.em
from optparse import OptionParser
import os
import sys

multiproc_exception = None
try:
    from multiprocessing import Pool
    # Detect whether we are running Windows Python via Wine. Wine does not
    # currently support some named pipe functions which the multiprocessing
    # module needs: http://bugs.winehq.org/show_bug.cgi?id=17273
    if sys.platform == 'win32' and 'WINELOADERNOEXEC' in os.environ:
        multiproc_exception = "Wine does not currently support multiprocessing"
except ImportError, detail:
    multiproc_exception = str(detail)

class Fitter(object):
    def __init__(self, em_map, spacing, resolution, origin, density_threshold,pdb, fits_fn, angle,num_fits,angles_per_voxel,max_trans,max_angle,ref_pdb=''):
        self.em_map = em_map
        self.spacing = spacing
        self.resolution = resolution
        self.threshold=density_threshold
        self.originx = origin[0]
        self.originy = origin[1]
        self.originz = origin[2]
        self.pdb = pdb
        self.fits_fn = fits_fn
        self.angle = angle
        self.num_fits=num_fits
        self.angles_per_voxel=angles_per_voxel
        self.max_trans=max_trans
        self.max_angle=max_angle
        self.ref_pdb=ref_pdb

    #TODO - update function
    def run_local_fitting(self,mol2fit,rb,initial_transformation):
        print "resolution is:",self.resolution
        dmap = IMP.em.read_map(self.em_map)
        dmap.get_header().set_resolution(self.resolution)
        dmap.update_voxel_size(self.spacing)
        dmap.set_origin(IMP.algebra.Vector3D(self.originx,
                                             self.originy,
                                             self.originz))
        dmap.set_was_used(True)
        dmap.get_header().show()
        mh_xyz=IMP.core.XYZs(IMP.core.get_leaves(mol2fit))
        ff = IMP.multifit.FFTFitting()
        ff.set_was_used(True)
        #####
        do_cluster_fits=True
        max_clustering_translation=3
        max_clustering_rotation=5
        num_fits_to_report=100
        #####
        fits = ff.do_local_fitting(dmap, self.threshold,mol2fit,
                                   self.angle / 180.0 * math.pi,
                                   self.max_angle / 180.0 * math.pi, self.max_trans, num_fits_to_report,
                                   do_cluster_fits, self.angles_per_voxel,
                                   max_clustering_translation,max_clustering_rotation)
        fits.set_was_used(True)
        final_fits = fits.best_fits_
        if self.ref_pdb!='':
            ref_mh=IMP.atom.read_pdb(self.ref_pdb,mdl)
            ref_mh_xyz=IMP.core.XYZs(IMP.core.get_leaves(ref_mh))
            cur_low=[1e4,0]
        for i, fit in enumerate(final_fits):
            fit.set_index(i)
            if self.ref_pdb!='':
                trans=fit.get_fit_transformation()
                IMP.atom.transform(mol2fit,trans)
                rmsd=IMP.atom.get_rmsd(mh_xyz,ref_mh_xyz)
                if rmsd<cur_low[0]:
                    cur_low[0]=rmsd
                    cur_low[1]=i
                fit.set_rmsd_to_reference(rmsd)
                IMP.atom.transform(mol2fit,trans.get_inverse())
            fit.set_fit_transformation(trans*initial_transformation)
        if self.ref_pdb!='':
            print 'from all fits, lowest rmsd to ref:',cur_low
        IMP.multifit.write_fitting_solutions(self.fits_fn, final_fits)



def do_work(f):
    f.run()

def parse_args():
    usage = """%prog [options] <assembly input> <refined assembly input> <proteomics.input> <mapping.input> <combinations file> <combination index>

Fit subunits locally around a combination solution with FFT."""
    parser = OptionParser(usage)
    parser.add_option("-c", "--cpu", dest="cpus", type="int", default=1,
                      help="number of cpus to use (default 1)")
    parser.add_option("-a", "--angle", dest="angle", type="float",
                      default=5,
                      help="angle delta (degrees) for FFT rotational "
                           "search (default 5)")

    parser.add_option("-n", "--num", dest="num", type="int",
                      default=100,
                      help="Number of fits to report"
                           "(default 100)")

    parser.add_option("-v", "--angle_voxel", dest="angle_voxel", type="int",
                      default=10,
                      help="Number of angles to keep per voxel"
                           "(default 10)")

    parser.add_option("-t", "--max_trans", dest="max_trans", type="float",
                      default=10.,
                      help="maximum translational search in A"
                          "(default 10)")

    parser.add_option("-m", "--max_angle", dest="max_angle", type="float",
                      default=30.,
                      help="maximum angular search in degrees"
                          "(default 50)")

    options, args = parser.parse_args()
    if len(args) != 6:
        parser.error("incorrect number of arguments")
    return options,args

def run(asmb_fn, asmb_refined_fn, proteomics_fn,mapping_fn,combs_fn,comb_ind,options):
    #get rmsd for subunits
    mdl1=IMP.Model()
    mdl2=IMP.Model()
    combs=IMP.multifit.read_paths(combs_fn)
    asmb_input=IMP.multifit.read_settings(asmb_fn)
    asmb_input.set_was_used(True)
    asmb_refined_input=IMP.multifit.read_settings(asmb_refined_fn)
    asmb_refined_input.set_was_used(True)
    prot_data=IMP.multifit.read_proteomics_data(proteomics_fn)
    mapping_data=IMP.multifit.read_protein_anchors_mapping(prot_data,
                                                           mapping_fn)
    ensmb=IMP.multifit.load_ensemble(asmb_input,mdl1,mapping_data)
    mhs=ensmb.get_molecules()

    ensmb_ref=IMP.multifit.load_ensemble(asmb_input,mdl2,mapping_data)
    mhs_ref=ensmb_ref.get_molecules()

    ensmb.load_combination(combs[comb_ind])

    em_map=asmb_input.get_assembly_header().get_dens_fn()
    resolution=asmb_input.get_assembly_header().get_resolution()
    spacing=asmb_input.get_assembly_header().get_spacing()
    origin=asmb_input.get_assembly_header().get_origin()

    rbs_ref=ensmb_ref.get_rigid_bodies()
    rbs=ensmb.get_rigid_bodies()

    for i,mh in enumerate(mhs):
        fits_fn=asmb_refined_input.get_component_header(i).get_transformations_fn()

        #todo - get the initial transformation
        rb_ref=rbs_ref[i]
        rb=rbs[i]

        initial_transformation=IMP.algebra.get_transformation_from_first_to_second(
                rb_ref.get_reference_frame(),
                rb.get_reference_frame())

        pdb_fn=asmb_input.get_component_header(i).get_filename()

        f = Fitter(em_map, spacing, resolution, origin, asmb_input.get_assembly_header().get_threshold(),pdb_fn, fits_fn, options.angle,options.num,options.angle_voxel,
                   options.max_trans,options.max_angle)
        if multiproc_exception is None and options.cpus > 1:
            work_units.append(f)
        else:
            if options.cpus > 1:
                options.cpus = 1
                print >> sys.stderr, """
The Python 'multiprocessing' module (available in Python 2.6 and later) is
needed to run on multiple CPUs, and could not be found
(Python error: '%s').
Running on a single processor.""" % multiproc_exception
            f.run_local_fitting(mh,rb,initial_transformation)
    if multiproc_exception is None and options.cpus > 1:
        # No point in spawning more processes than components
        nproc = min(options.cpus, asmb_input.get_number_of_component_headers())
        p = Pool(processes=nproc)
        out = list(p.imap_unordered(do_work, work_units))

def main():
    options,args = parse_args()
    asmb_input = args[0]
    asmb_refined_input = args[1]
    proteomics_fn=args[2]
    mapping_fn=args[3]
    combinations_fn = args[4]
    combination_ind = int(args[5])
    run(asmb_input, asmb_refined_input, proteomics_fn,mapping_fn,combinations_fn, combination_ind, options)

if __name__=="__main__":
    main()
