# This example demonstrates how non-polymeric entities (ligands, water)
# are handled by the Python IHM library. See the simple-docking.py example
# for an introduction to the library.

import ihm
import ihm.dumper
import ihm.protocol
import ihm.representation
import ihm.model

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

# Just as in the simple-docking.py example, we can add models with coordinates.
# Here we define an atomic model containing just the two hemes and the water.
assembly = ihm.Assembly((asym_heme1, asym_heme2, asym_h2o),
                        name="Modeled assembly")
rep = ihm.representation.Representation(
    [ihm.representation.AtomicSegment(asym_heme1, rigid=False),
     ihm.representation.AtomicSegment(asym_heme2, rigid=False),
     ihm.representation.AtomicSegment(asym_h2o, rigid=False)])
protocol = ihm.protocol.Protocol(name='Modeling')


class MyModel(ihm.model.Model):
    def get_atoms(self):
        # seq_id only makes sense for polymers; for ligands it should be None
        yield ihm.model.Atom(asym_unit=asym_heme1, type_symbol='FE', het=True,
                             seq_id=None, atom_id='FE', x=0., y=0., z=0.)
        yield ihm.model.Atom(asym_unit=asym_heme2, type_symbol='FE', het=True,
                             seq_id=None, atom_id='FE', x=10., y=10., z=10.)
        yield ihm.model.Atom(asym_unit=asym_h2o, type_symbol='O', het=True,
                             seq_id=None, atom_id='O', x=20., y=20., z=20.)


# We have only a single model in a single state:
model = MyModel(assembly=assembly, protocol=protocol, representation=rep,
                name='Best model')
model_group = ihm.model.ModelGroup([model], name='All models')
state = ihm.model.State([model_group])
system.state_groups.append(ihm.model.StateGroup([state]))

# Once the system is complete, we can write it out to an mmCIF file:
with open('output.cif', 'w') as fh:
    ihm.dumper.write(fh, [system])
