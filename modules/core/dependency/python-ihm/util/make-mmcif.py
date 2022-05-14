#!/usr/bin/env python3

"""
Add minimal IHM-related tables to an mmCIF file.

Given any mmCIF file as input, this script will add any missing
IHM-related tables and write out a new file that is minimally compliant
with the IHM dictionary.

This is done by simply reading in the original file with python-ihm and
then writing it out again, so
  a) any data in the input file that is not understood by python-ihm
     will be lost on output; and
  b) input files that aren't compliant with the PDBx dictionary, or that
     contain syntax errors or other problems may crash or otherwise confuse
     python-ihm.
"""


import ihm.reader
import ihm.dumper
import ihm.model
import ihm.protocol
import sys


def add_ihm_info(s):
    if not s.title:
        s.title = 'Auto-generated system'

    # Simple default assembly containing all chains
    default_assembly = ihm.Assembly(s.asym_units, name='Modeled assembly')

    # Simple default atomic representation for everything
    default_representation = ihm.representation.Representation(
        [ihm.representation.AtomicSegment(asym, rigid=False)
         for asym in s.asym_units])

    # Simple default modeling protocol
    default_protocol = ihm.protocol.Protocol(name='modeling')

    for state_group in s.state_groups:
        for state in state_group:
            for model_group in state:
                for model in model_group:
                    if not model.assembly:
                        model.assembly = default_assembly
                    if not model.representation:
                        model.representation = default_representation
                    if not model.protocol:
                        model.protocol = default_protocol
    return s


if len(sys.argv) != 2:
    print("Usage: %s input.cif" % sys.argv[0], file=sys.stderr)
    sys.exit(1)
fname = sys.argv[1]

with open(fname) as fh:
    with open('output.cif', 'w') as fhout:
        ihm.dumper.write(
            fhout, [add_ihm_info(s) for s in ihm.reader.read(fh)],
            variant=ihm.dumper.IgnoreVariant(['_audit_conform']))
