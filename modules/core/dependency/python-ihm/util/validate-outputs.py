#!/usr/bin/python3

"""Check the output of each example for validity against the PDBx
   and IHM dictionaries.

   This should be periodically rechecked in case the PDBx and IHM dictionaries
   are updated.
"""

import sys
import os
import subprocess
import ihm.dictionary
import urllib.request

with urllib.request.urlopen(
        'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_pdbx_v50.dic') as fh:
    d_pdbx = ihm.dictionary.read(fh)
with urllib.request.urlopen(
        'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_ihm.dic') as fh:
    d_ihm = ihm.dictionary.read(fh)
pdbx_ihm = d_pdbx + d_ihm

for script in ('simple-docking.py', 'ligands_water.py',
               'non_standard_residues.py'):
    print(script)
    subprocess.check_call([sys.executable, '../examples/' + script])
    with open('output.cif') as fh:
        try:
            pdbx_ihm.validate(fh)
        except ihm.dictionary.ValidatorError as exc:
            print(exc)
os.unlink('output.cif')
