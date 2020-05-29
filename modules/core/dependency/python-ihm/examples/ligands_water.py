# This example demonstrates how non-polymeric entities (ligands, water)
# are handled by the Python IHM library. See the simple-docking.py example
# for an introduction to the library.

import ihm
import ihm.dumper

system = ihm.System()

# An entity corresponding to an amino acid (polyalanine) sequence
entity_protein = ihm.Entity('AAA', description='Subunit A')

# An entity corresponding to an RNA sequence
entity_rna = ihm.Entity('ACG', alphabet=ihm.RNAAlphabet,
                        description='RNA chain')

# An entity corresponding to a DNA sequence
entity_dna = ihm.Entity(['DA', 'DC', 'DG'], alphabet=ihm.DNAAlphabet,
                        description='DNA chain')

# Non-polymers such as ligands or water should each live in their own Entity:

# A ligand entity (in this case, heme)
heme = ihm.NonPolymerChemComp("HEM", name='PROTOPORPHYRIN IX CONTAINING FE',
                              formula='C34 H32 Fe N4 O4')
entity_heme = ihm.Entity([heme], description='Heme')

# Water
entity_h2o = ihm.Entity([ihm.WaterChemComp()], description='Water')

system.entities.extend((entity_protein, entity_rna, entity_dna, entity_heme,
                        entity_h2o))

# Next, we define asymmetric units for everything we modeled.
# Here, we have a single instance of each protein, RNA and DNA, two hemes,
# plus crystal waters
asym_protein = ihm.AsymUnit(entity_protein, details='Subunit A')
asym_rna = ihm.AsymUnit(entity_rna, details='RNA chain')
asym_dna = ihm.AsymUnit(entity_dna, details='DNA chain')
asym_heme1 = ihm.AsymUnit(entity_heme, details='First heme')
asym_heme2 = ihm.AsymUnit(entity_heme, details='Second heme')
asym_h2o = ihm.AsymUnit(entity_h2o, details='Crystal waters')
system.asym_units.extend((asym_protein, asym_rna, asym_dna, asym_heme1,
                          asym_heme2, asym_h2o))

# todo: show handling of multiple waters

# Once the system is complete, we can write it out to an mmCIF file:
with open('output.cif', 'w') as fh:
    ihm.dumper.write(fh, [system])
