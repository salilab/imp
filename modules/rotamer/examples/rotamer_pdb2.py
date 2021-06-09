## \example rotamer/rotamer_pdb2.py
# rotamer_pdb.py is a script demonstrating the usage of
# RotamerCalculator and RotamerLibrary.  It reads a PDB file and a
# rotamer library file, and tries to rotate the atoms based on the
# most probable chi angles from the rotamer library. Then it saves
# the rotated atoms to a specified output PDB file.
#
# Usage:
#
# `python rotamer_pdb.py -i <input>.pdb -l <rotamer_library>.lib -o <output>.pdb`
#
# Example (the result will be saved into transformed_1z5s_A.pdb):
#
# `python rotamer_pdb.py -i ../../atom/test/input/1z5s_A.pdb \
# -l /path/to/ALL.bbdep.rotamers.lib -o transformed_1z5s_A.pdb`
#

#!/usr/bin/env python

from __future__ import print_function
import IMP
import IMP.core
import IMP.atom
import IMP.algebra
import IMP.rotamer


def transform(input_pdb, input_lib, output_pdb):
    # read rotamer library
    rl = IMP.rotamer.RotamerLibrary()
    rl.read_library_file(input_lib)
    rc = IMP.rotamer.RotamerCalculator(rl)

    # read the original PDB
    m = IMP.Model()
    orig_h = IMP.atom.read_pdb(input_pdb, m)
    # mh = IMP.atom.get_by_type(orig_h, IMP.atom.RESIDUE_TYPE)

    # transform...
    hps = IMP.core.HarmonicDistancePairScore(1, 100)
    rc.transform(orig_h, hps, 0.9, 1e-6, 6)

    # save the rotated atoms to output PDB
    IMP.atom.write_pdb(orig_h, output_pdb)


def quick_test():
    rl = IMP.rotamer.RotamerLibrary()
    rc = IMP.rotamer.RotamerCalculator(rl)
    rc.set_was_used(True)


if __name__ == '__main__':

    import sys

    P = IMP.ArgumentParser()
    P.add_argument('--input_pdb', '-i', action='store',
                   help='input PDB file (required)')
    P.add_argument('--input_lib', '-l', action='store',
                   help='input rotamer library file (required)')
    P.add_argument('--output_pdb', '-o', action='store',
                   help='output PDB file (required)')
    P.add_argument('--verbose', '-v', action='store_true',
                   help='show more messages')
    args = P.parse_args()
    if IMP.get_bool_flag('run_quick_test') or \
       not (args.input_pdb or args.input_lib or args.output_pdb):
        quick_test()
        sys.exit(0)
    if not args.input_pdb:
        print('--input_pdb is required')
        sys.exit(1)
    if not args.output_pdb:
        print('--output_pdb is required')
        sys.exit(1)
    if not args.input_lib:
        print('--input_lib is required')
        sys.exit(1)
    if args.verbose:
        IMP.set_log_level(IMP.VERBOSE)
    else:
        IMP.set_log_level(IMP.SILENT)
    transform(args.input_pdb, args.input_lib, args.output_pdb)
