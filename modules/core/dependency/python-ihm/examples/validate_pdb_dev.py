# This example demonstrates the use of the Python IHM library's validator.
# A structure is downloaded from the PDB-Dev database and checked against
# the PDBx and IHM dictionaries for compliance. This validator can be used
# to perform basic integrity checking against any mmCIF dictionary.

import ihm.reader
import ihm.dictionary
try:
    import urllib.request as urllib2  # Python 3
except ImportError:
    import urllib2  # Python 2

# Read in the PDBx dictionary from wwPDB as a Dictionary object
fh = urllib2.urlopen(
    'http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_pdbx_v50.dic')
d_pdbx = ihm.dictionary.read(fh)
fh.close()

# Also read in the IHM dictionary
fh = urllib2.urlopen('http://mmcif.wwpdb.org/dictionaries/ascii/mmcif_ihm.dic')
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
# Here, a structure from PDB-Dev (which should be valid) is used.
fh = urllib2.urlopen('https://pdb-dev.wwpdb.org/cif/PDBDEV_00000001.cif')
pdbx_ihm.validate(fh)
fh.close()
