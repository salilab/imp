# This example demonstrates how to add non-standard residues to
# sequences handled by the Python IHM library. See the simple-docking.py
# example for an introduction to the library.

import ihm
import ihm.dumper

system = ihm.System()

# IHM contains definitions for standard amino and nucleic acids, plus
# a few common non-standard residues such as MSE and UNK.
# To create a new non-standard residue, we first need to create a chemical
# component for it. In this case, we add a definition for norvaline, an
# isomer of valine. IHM provides a ChemComp class for this purpose and a
# number of more specialized subclasses. Since norvaline is a chiral peptide,
# here we define it in its L- form using the LPeptideChemComp class.
#
# 'id' should match the officially defined name of the component, as defined
#     in the chemical component dictionary: https://www.wwpdb.org/data/ccd
#     (See also https://www3.rcsb.org/ligand/NVA)
# 'code' is used to populate the primary sequence in the output mmCIF file.
#     For non-standard residues it should normally match 'id'.
# 'code_canonical' is the one-letter code of the closest standard residue.
#     Here we use 'V', valine.
norvaline = ihm.LPeptideChemComp(id='NVA', code='NVA', code_canonical='V',
                                 name='NORVALINE', formula='C5 H11 N O2')

# The Entity constructor takes a sequence of either or both one-letter codes
# and ChemComp objects, so now we can make a sequence containing both
# alanine and norvaline:
entity1 = ihm.Entity(['A', 'A', norvaline, 'A'], description='First entity')


# If a particular non-standard residue is commonly used in your own software,
# and you have assigned a one-letter code for it, you can subclass
# the ihm Alphabet class appropriately. Here we extend the normal set of
# one-letter codes (uppercase) for standard L- amino acids to add 'n' for
# norvaline:
class MyAlphabet(ihm.LPeptideAlphabet):
    # Alphabet contains a _comps dictionary that is a simple mapping from
    # codes (usually one-letter) to ChemComp objects
    _comps = {}
    _comps.update(ihm.LPeptideAlphabet._comps)
    _comps['n'] = norvaline


# Now we can pass a primary sequence using our custom alphabet to include
# norvaline alongside standard cysteine:
entity2 = ihm.Entity('CCnC', alphabet=MyAlphabet, description="Second entity")

system.entities.extend((entity1, entity2))

# Once the system is complete, we can write it out to an mmCIF file:
with open('output.cif', 'w') as fh:
    ihm.dumper.write(fh, [system])
