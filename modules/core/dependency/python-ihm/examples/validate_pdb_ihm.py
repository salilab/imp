# This example demonstrates the use of the Python IHM library's validator.
# A structure is downloaded from the PDB-IHM database and checked against
# the PDBx and IHM dictionaries for compliance. This validator can be used
# to perform basic integrity checking against any mmCIF dictionary; for an
# example of using it to validate homology models against the ModelCIF
# dictionary, see
# https://github.com/ihmwg/python-modelcif/blob/main/examples/validate_modbase.py.

import io
import ihm.reader
import ihm.dictionary
import urllib.request

# Read in the PDBx dictionary from wwPDB as a Dictionary object
fh = urllib.request.urlopen(
    'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_pdbx_v50.dic')
d_pdbx = ihm.dictionary.read(fh)
fh.close()

# Also read in the IHM dictionary
fh = urllib.request.urlopen(
    'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_ihm.dic')
d_ihm = ihm.dictionary.read(fh)
fh.close()

# Deposited integrative models should conform to both the PDBx dictionary
# (used to define basic structural information such as residues and chains)
# and the IHM dictionary (used for information specific to integrative
# modeling). Make a dictionary that combines the PDBx and IHM dictionaries
# using the + operator.
pdbx_ihm = d_pdbx + d_ihm

# Validate a structure against PDBx+IHM.
# A correct structure here should result in no output; an invalid structure
# will result in a ValidatorError Python exception.
# Here, a structure from PDB-IHM (which should be valid) is used.
acc = '8zz1'
cif = urllib.request.urlopen('https://pdb-ihm.org/cif/%s.cif' % acc).read()

# The encoding for mmCIF files isn't strictly defined, so first try UTF-8
# and if that fails, strip out any non-ASCII characters. This ensures that
# we handle accented characters in string fields correctly.
try:
    fh = io.StringIO(cif.decode('utf-8'))
except UnicodeDecodeError:
    fh = io.StringIO(cif.decode('ascii', errors='ignore'))

pdbx_ihm.validate(fh)
