#!/usr/bin/python3

"""Get the set of canonical atom names for one or more residue types,
   by querying CCD via RCSB. Used to populate the KNOWN_ATOM_NAMES variable
   in make_mmcif.py.
"""

import collections
import ihm.format
import urllib.request
import sys


class ChemCompAtomHandler:
    not_in_file = omitted = unknown = None

    def __init__(self):
        super().__init__()
        self.atoms = collections.defaultdict(set)

    def __call__(self, comp_id, atom_id):
        self.atoms[comp_id].add(atom_id)


cca = ChemCompAtomHandler()
for resname in sys.argv[1:]:
    with urllib.request.urlopen(
            'https://files.rcsb.org/pub/pdb/refdata/chem_comp/%s/%s/%s.cif'
            % (resname[-1], resname, resname)) as fh:
        c = ihm.format.CifReader(fh, category_handler={'_chem_comp_atom': cca})
        c.read_file()

print("KNOWN_ATOM_NAMES = {")
for comp_id in sorted(cca.atoms.keys()):
    print('    %r: %r,' % (comp_id, cca.atoms[comp_id]))
print("}")
