# This example demonstrates the use of the Python IHM library to generate
# an mmCIF file for a very simple integrative docking study. Two subunits,
# A and B, each of which is fitted against small angle X-ray (SAXS) data, are
# docked together into a complex, AB, which is fitted against an electron
# microscopy density map.

import ihm
import ihm.location
import ihm.dataset
import ihm.representation
import ihm.restraint
import ihm.protocol
import ihm.model
import ihm.dumper

# First, we create a system, which contains everything we know about the
# modeling. A single mmCIF file can contain multiple Systems, but in most
# cases we use just one:
system = ihm.System()

# Next, we describe the input data we used, using dataset classes.
# Each source of data has a location, such as a file on disk or a database
# entry, and a type. In this example we used EM density data, which we'll
# say lives in the EMDB database:
loc = ihm.location.EMDBLocation('EMDB-1234')
em_dataset = ihm.dataset.EMDensityDataset(loc)
# We also used two SAXS profiles, which we'll say live in SASBDB:
saxsA_dataset = ihm.dataset.SASDataset(ihm.location.SASBDBLocation('SASDB123'))
saxsB_dataset = ihm.dataset.SASDataset(ihm.location.SASBDBLocation('SASDB456'))

# Where datasets are derived from some other data, it is helpful to also point
# back to that primary data. In this case, let's say the EM density was
# derived from a set of EM micrographs, deposited in the EMPIAR database:
m = ihm.dataset.EMMicrographsDataset(ihm.location.EMPIARLocation('EMPIAR-123'))
em_dataset.parents.append(m)

# Next, define the entities for each unique sequence in the system
# (here represented as polyalanines):
entityA = ihm.Entity('AAA', description='Subunit A')
entityB = ihm.Entity('AAAAAA', description='Subunit B')
system.entities.extend((entityA, entityB))

# Next, we define asymmetric units for everything we modeled.
# These roughly correspond to chains in a traditional PDB file. Multiple
# asymmetric units may map to the same entity (for example if there are
# several copies of a given protein). Parts of the system that were seen in
# an experiment but were not modeled are represented as entities to which no
# asymmetric units map.
asymA = ihm.AsymUnit(entityA, details='Subunit A')
asymB = ihm.AsymUnit(entityB, details='Subunit B')
system.asym_units.extend((asymA, asymB))

# Next, we group asymmetric units (and/or entities) into assemblies.
# Here, we'll define an assembly of everything that we modeled, plus
# two subassemblies (of the subunits) that the SAXS data applies to:
modeled_assembly = ihm.Assembly((asymA, asymB), name='Modeled assembly')
assemblyA = ihm.Assembly((asymA,), name='Subunit A')
assemblyB = ihm.Assembly((asymB,), name='Subunit B')

# Define how the system was represented. Multiple representations of the
# system are possible, and can overlap. Here we'll say we represent A
# atomically as a rigid body and B as 3 flexible coarse-grained spheres:
rep = ihm.representation.Representation(
    [ihm.representation.AtomicSegment(asymA, rigid=True),
     ihm.representation.FeatureSegment(asymB, rigid=False,
                                       primitive='sphere', count=3)])

# Set up restraints on the system. First, two on the subunits that use
# the SAXS data; we'll say we used the FoXS software to do this fit:
saxsA_rsr = ihm.restraint.SASRestraint(
    dataset=saxsA_dataset, assembly=assemblyA,
    fitting_method='FoXS', fitting_atom_type='Heavy atoms')
saxsB_rsr = ihm.restraint.SASRestraint(
    dataset=saxsB_dataset, assembly=assemblyB,
    fitting_method='FoXS', fitting_atom_type='Heavy atoms')
system.restraints.extend((saxsA_rsr, saxsB_rsr))
# Next, the EM restraint applied to the entire system:
em_rsr = ihm.restraint.EM3DRestraint(
    dataset=em_dataset, assembly=modeled_assembly)
system.restraints.append(em_rsr)

# Now we add information about how the modeling was done by defining one
# or more protocols. Here we'll say we did simple Monte Carlo on the entire
# system using all of the experimental data:
all_datasets = ihm.dataset.DatasetGroup((em_dataset, saxsA_dataset,
                                         saxsB_dataset))
protocol = ihm.protocol.Protocol(name='Modeling')
protocol.steps.append(ihm.protocol.Step(
    assembly=modeled_assembly, dataset_group=all_datasets,
    method='Monte Carlo', name='Production sampling',
    num_models_begin=0, num_models_end=1000, multi_scale=True))

# Finally we can add coordinates for the deposited models. Typically these
# will be stored in our own software's data structures somewhere (for this
# example in simple lists 'atoms' and 'spheres'):
atoms = [('A', 1, 'C', 'CA', 1., 2., 3.),
         ('A', 2, 'C', 'CA', 4., 5., 6.),
         ('A', 3, 'C', 'CA', 7., 8., 9.)]
spheres = [('B', 1, 2, 1., 2., 3., 1.2),
           ('B', 3, 4, 4., 5., 6., 1.2),
           ('B', 5, 6, 7., 8., 9., 1.2)]


# Rather than storing another copy of the coordinates in the IHM library
# (which could use a lot of memory), we need to provide a mechanism to
# translate them into the IHM data model. We do this straightforwardly by
# subclassing the IHM Model class and overriding the get_atoms
# and get_spheres methods:
class MyModel(ihm.model.Model):
    # Map our asym unit names A and B to IHM asym_unit objects:
    asym_unit_map = {'A': asymA, 'B': asymB}

    def get_atoms(self):
        for asym, seq_id, type_symbol, atom_id, x, y, z in atoms:
            yield ihm.model.Atom(asym_unit=self.asym_unit_map[asym],
                                 type_symbol=type_symbol, seq_id=seq_id,
                                 atom_id=atom_id, x=x, y=y, z=z)

    def get_spheres(self):
        for asym, seq_id_start, seq_id_end, x, y, z, radius in spheres:
            yield ihm.model.Sphere(asym_unit=self.asym_unit_map[asym],
                                   seq_id_range=(seq_id_start, seq_id_end),
                                   x=x, y=y, z=z, radius=radius)


model = MyModel(assembly=modeled_assembly, protocol=protocol,
                representation=rep, name='Best scoring model')

# Note that the model was scored against all three restraints
saxsA_rsr.fits[model] = ihm.restraint.SASRestraintFit(chi_value=1.4)
saxsB_rsr.fits[model] = ihm.restraint.SASRestraintFit(chi_value=2.1)
em_rsr.fits[model] = ihm.restraint.EM3DRestraintFit(
    cross_correlation_coefficient=0.9)

# Similar models can be grouped together. Here we only have a single model
# in the group
model_group = ihm.model.ModelGroup([model], name='All models')

# Groups are then placed into states, which can in turn be grouped. In this
# case we have only a single state:
state = ihm.model.State([model_group])
system.state_groups.append(ihm.model.StateGroup([state]))

# Once the system is complete, we can write it out to an mmCIF file:
with open('output.cif', 'w') as fh:
    ihm.dumper.write(fh, [system])
