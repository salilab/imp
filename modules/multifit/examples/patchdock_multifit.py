#!/usr/bin/python

import optparse
import sys
import IMP
import IMP.multifit


parser = optparse.OptionParser()
parser.add_option('--patchdock_binary', '-b', action='store', type='string',
    help='path to patchdock binary (required)')
parser.add_option('--pdb_file', '-p', action='store', type='string',
    help='pdb file (required)')
parser.add_option('--ms_file', '-m', action='store', type='string',
    help='MS surface file (required)')
parser.add_option('--prot_lib_file', '-l', action='store', type='string',
    help='prot lib file (required)')
parser.add_option('--output_file', '-o', action='store', type='string',
    help='output file (required)')
parser.add_option('--density_map_file', '-d', action='store', type='string',
    help='density map file (required)')
parser.add_option('--chimera_file', '-C', action='store', type='string',
    help='Chimera output file (optional)')
parser.add_option('--verbose', '-v', action='store_true',
    help='print lots of messages')
opts, args = parser.parse_args()
if not opts.patchdock_binary:
    print >> sys.stderr, '--patchdock_binary is required'
    sys.exit(0)
if not opts.pdb_file:
    print >> sys.stderr, '--pdb_file is required'
    sys.exit(1)
if not opts.ms_file:
    print >> sys.stderr, '--ms_file is required'
    sys.exit(1)
if not opts.prot_lib_file:
    print >> sys.stderr, '--prot_lib_file is required'
    sys.exit(1)
if not opts.output_file:
    print >> sys.stderr, '--output_file is required'
    sys.exit(1)
if not opts.density_map_file:
    print >> sys.stderr, '--density_map_file is required'
    sys.exit(1)
if opts.chimera_file:
    chimera = opts.chimera_file
else:
    chimera = ""

if opts.verbose:
    IMP.set_log_level(IMP.VERBOSE)
else:
    IMP.set_log_level(IMP.SILENT)

SF = IMP.multifit.SymmetricFit()
SF.set_patchdock_executable(opts.patchdock_binary)
SF.set_protein_pdb_file(opts.pdb_file)
SF.set_protein_ms_file(opts.ms_file)
SF.set_prot_lib_file(opts.prot_lib_file)
SF.set_density_map_file(opts.density_map_file)
SF.run(opts.output_file, chimera)
