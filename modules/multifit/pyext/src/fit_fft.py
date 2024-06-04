#!/usr/bin/env python

from __future__ import print_function
import math
import IMP.multifit
import IMP.atom
import IMP.em
from IMP import ArgumentParser
import os
import sys

__doc__ = "Fit subunits into a density map with FFT."

multiproc_exception = None
try:
    import multiprocessing
    # Detect whether we are running Windows Python via Wine. Wine does not
    # currently support some named pipe functions which the multiprocessing
    # module needs: http://bugs.winehq.org/show_bug.cgi?id=17273
    if sys.platform == 'win32' and 'WINELOADERNOEXEC' in os.environ:
        multiproc_exception = "Wine does not currently support multiprocessing"
except ImportError as detail:
    multiproc_exception = str(detail)


def _get_context():
    # Use 'forkserver' rather than 'fork' start method if we can; 'fork' does
    # not work well with multithreaded processes or CUDA
    if (hasattr(multiprocessing, 'get_all_start_methods')
            and 'forkserver' in multiprocessing.get_all_start_methods()):
        return multiprocessing.get_context('forkserver')
    else:
        return multiprocessing.get_context()


class Fitter(object):

    def __init__(
        self,
        em_map,
        spacing,
        resolution,
        origin,
        density_threshold,
        pdb,
        fits_fn,
        angle,
        num_fits,
        angles_per_voxel,
            ref_pdb=''):
        self.em_map = em_map
        self.spacing = spacing
        self.resolution = resolution
        self.threshold = density_threshold
        self.originx = origin[0]
        self.originy = origin[1]
        self.originz = origin[2]
        self.pdb = pdb
        self.fits_fn = fits_fn
        self.angle = angle
        self.num_fits = num_fits
        self.angles_per_voxel = angles_per_voxel
        self.ref_pdb = ref_pdb

    def run(self):
        print("resolution is:", self.resolution)
        dmap = IMP.em.read_map(self.em_map)
        dmap.get_header().set_resolution(self.resolution)
        dmap.update_voxel_size(self.spacing)
        dmap.set_origin(IMP.algebra.Vector3D(self.originx,
                                             self.originy,
                                             self.originz))
        dmap.set_was_used(True)
        dmap.get_header().show()
        mdl = IMP.Model()
        mol2fit = IMP.atom.read_pdb(self.pdb, mdl)
        mh_xyz = IMP.core.XYZs(IMP.core.get_leaves(mol2fit))
        _ = IMP.atom.create_rigid_body(mol2fit)
        ff = IMP.multifit.FFTFitting()
        ff.set_was_used(True)
        fits = ff.do_global_fitting(dmap, self.threshold, mol2fit,
                                    self.angle / 180.0 * math.pi,
                                    self.num_fits, self.spacing, 0.5,
                                    True, self.angles_per_voxel)
        fits.set_was_used(True)
        final_fits = fits.best_fits_
        if self.ref_pdb != '':
            ref_mh = IMP.atom.read_pdb(self.ref_pdb, mdl)
            ref_mh_xyz = IMP.core.XYZs(IMP.core.get_leaves(ref_mh))
            cur_low = [1e4, 0]
        for i, fit in enumerate(final_fits):
            fit.set_index(i)
            if self.ref_pdb != '':
                trans = fit.get_fit_transformation()
                IMP.atom.transform(mol2fit, trans)
                rmsd = IMP.atom.get_rmsd(mh_xyz, ref_mh_xyz)
                if rmsd < cur_low[0]:
                    cur_low[0] = rmsd
                    cur_low[1] = i
                fit.set_rmsd_to_reference(rmsd)
                IMP.atom.transform(mol2fit, trans.get_inverse())
        if self.ref_pdb != '':
            print('from all fits, lowest rmsd to ref:', cur_low)
        IMP.multifit.write_fitting_solutions(self.fits_fn, final_fits)


def do_work(f):
    f.run()


def parse_args():
    desc = """Fit subunits into a density map with FFT."""
    p = ArgumentParser(description=desc)
    p.add_argument("-c", "--cpu", dest="cpus", type=int, default=1,
                   help="number of cpus to use (default 1)")
    p.add_argument("-a", "--angle", dest="angle", type=float, default=30,
                   help="angle delta (degrees) for FFT rotational "
                        "search (default 30)")

    p.add_argument("-n", "--num", dest="num", type=int,
                   default=100, help="Number of fits to report (default 100)")

    p.add_argument("-v", "--angle_voxel", dest="angle_voxel", type=int,
                   default=10,
                   help="Number of angles to keep per voxel (default 10)")

    p.add_argument("assembly_file", help="assembly file name")

    # p.add_argument("-n", "--num", dest="num", type="int",
    #                  default=100,
    #                  help="Number of fits to report"
    #                      "(default 100)")

    return p.parse_args()


def run(asmb_fn, options):
    if multiproc_exception is None and options.cpus > 1:
        work_units = []
    asmb_input = IMP.multifit.read_settings(asmb_fn)
    asmb_input.set_was_used(True)
    em_map = asmb_input.get_assembly_header().get_dens_fn()
    resolution = asmb_input.get_assembly_header().get_resolution()
    spacing = asmb_input.get_assembly_header().get_spacing()
    origin = asmb_input.get_assembly_header().get_origin()
    for i in range(asmb_input.get_number_of_component_headers()):
        fits_fn = asmb_input.get_component_header(i).get_transformations_fn()
        pdb_fn = asmb_input.get_component_header(i).get_filename()
        f = Fitter(
            em_map,
            spacing,
            resolution,
            origin,
            asmb_input.get_assembly_header().get_threshold(),
            pdb_fn,
            fits_fn,
            options.angle,
            options.num,
            options.angle_voxel)
        if multiproc_exception is None and options.cpus > 1:
            work_units.append(f)
        else:
            if options.cpus > 1:
                options.cpus = 1
                print("""
The Python 'multiprocessing' module (available in Python 2.6 and later) is
needed to run on multiple CPUs, and could not be found
(Python error: '%s').
Running on a single processor.""" % multiproc_exception, file=sys.stderr)
            f.run()
    if multiproc_exception is None and options.cpus > 1:
        # No point in spawning more processes than components
        nproc = min(options.cpus, asmb_input.get_number_of_component_headers())
        ctx = _get_context()
        p = ctx.Pool(processes=nproc)
        _ = list(p.imap_unordered(do_work, work_units))
        p.close()


def main():
    args = parse_args()
    run(args.assembly_file, args)


if __name__ == "__main__":
    main()
