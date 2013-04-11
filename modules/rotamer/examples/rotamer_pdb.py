## \example rotamer/rotamer_pdb.py
## rotamer_pdb.py is a script demonstrating the usage of RotamerCalculator and RotamerLibrary.
## It reads a PDB file and a rotamer library file, and tries to rotate the atoms based on the most
## probable chi angles from the rotamer library. Then it saves the rotated atoms to a specified output
## PDB file.
##
## Usage:
##
## `python rotamer_pdb.py -i <input>.pdb -l <rotamer_library>.lib -o <output>.pdb`
##
## Example (the result will be saved into transformed_1z5s_A.pdb):
##
## `../../../tools/imppy.sh python rotamer_pdb.py -i ../../atom/test/input/1z5s_A.pdb \
##   -l /path/to/ALL.bbdep.rotamers.lib -o transformed_1z5s_A.pdb`
##

#!/usr/bin/env python

import IMP
import IMP.core
import IMP.atom
import IMP.algebra
import IMP.rotamer


def transform(input_pdb, input_lib, output_pdb):
    # read the original PDB
    m = IMP.Model()
    orig_h = IMP.atom.read_pdb(input_pdb, m)
    mh = IMP.atom.get_by_type(orig_h, IMP.atom.RESIDUE_TYPE)

    # read rotamer library
    rl = IMP.rotamer.RotamerLibrary()
    rl.read_library_file(input_lib)
    rc = IMP.rotamer.RotamerCalculator(rl)

    # get the most probable rotamers
    rotamers = list()
    for h in mh:
        rd = h.get_as_residue()
        rr = rc.get_rotamer(rd, 0.01)
        rotamers.append((rd, rr))

    # now set the coordinates of all atoms in the residues to the rotated coordinates
    for rd, rr in rotamers:
        for h in IMP.atom.get_by_type(rd, IMP.atom.ATOM_TYPE):
            at = h.get_as_atom()
            at_t = at.get_atom_type()
            if rr.get_atom_exists(at_t):
                # some atoms might not be rotated
                idx = min(rr.get_number_of_cases(at_t) - 1, 1)
                v = rr.get_coordinates(idx, at_t)
                xyz = IMP.core.XYZ(at)
                xyz.set_coordinates(v)

    # save the rotated atoms to output PDB
    IMP.atom.write_pdb(orig_h, output_pdb)


def quick_test():
    rl = IMP.rotamer.RotamerLibrary()
    rc = IMP.rotamer.RotamerCalculator(rl)


if __name__ == '__main__':

    import sys
    import optparse

    P = optparse.OptionParser()
    P.add_option('--input_pdb', '-i', action='store', type='string',
        help='input PDB file (required)')
    P.add_option('--input_lib', '-l', action='store', type='string',
        help='input rotamer library file (required)')
    P.add_option('--output_pdb', '-o', action='store', type='string',
        help='output PDB file (required)')
    P.add_option('--verbose', '-v', action='store_true',
        help='show more messages')
    P.add_option('--run_quick_test', action='store_true',
        help='run quick test')
    opts, args = P.parse_args()
    if opts.run_quick_test:
        quick_test()
        sys.exit(0)
    if not opts.input_pdb:
        print '--input_pdb is required'
        sys.exit(1)
    if not opts.output_pdb:
        print '--output_pdb is required'
        sys.exit(1)
    if not opts.input_lib:
        print '--input_lib is required'
        sys.exit(1)
    if opts.verbose:
        IMP.base.set_log_level(IMP.base.VERBOSE)
    else:
        IMP.base.set_log_level(IMP.base.SILENT)
    transform(opts.input_pdb, opts.input_lib, opts.output_pdb)
