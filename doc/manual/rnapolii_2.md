Stage 2 - Representation of subunits and translation of the data into spatial restraints {#rnapolii_2}
========================================================================================

In this stage, we will initially define a representation of the system. Afterwards, we will convert the data into spatial restraints.  This is performed using the script `rnapolii/modeling/modeling.py` and uses the
[topology file](@ref IMP::pmi::topology::TopologyReader),
`topology.txt`, to define the system components and their representation
parameters.

### Setting up Model Representation in IMP

**Representation** 
Very generally, the *representation* of a system is defined by all the variables that need to be determined based on input information, including the assignment of the system components to geometric objects (e.g. points, spheres, ellipsoids, and 3D Gaussian density functions). 

Our RNA Pol II representation employs *spherical beads* of varying sizes and *3D Gaussians*, which coarsen domains of the complex using several resolution scales simultaneously. The *spatial restraints* will be applied to individual resolution scales as appropriate. 

Beads and Gaussians of a given domain are arranged into either a rigid body or a flexible string, based on the crystallographic structures. In a *rigid body*, all the beads and the Gaussians of a given domain have their relative distances constrained during configurational sampling, while in a *flexible string* the beads and the Gaussians are restrained by the sequence connectivity. 

<img src="rnapolii_Multi-scale_representation.png" width="600px" />
_Multi-scale representation of Rpb1 subunit of RNA Pol II_



The GMM of a subunit is the set of all 3D Gaussians used to represent it; it will be used to calculate the EM score. The calculation of the GMM of a subunit can be done automatically in the
[topology file](@ref IMP::pmi::topology::TopologyReader).
For the purposes of this tutorial, we already created these for Rpb4 and Rpb7 and placed them in the `rnapolii/data` directory in their respective `.mrc` and `.txt` files. 

**Dissecting the script**
The script `rnapolii/modeling/modeling.py` sets up the representation of the system and the restraint. (Subsequently it also performs [sampling](@ref rnapolii_3), but more on that later.)

**Header**  
The first part of the script defines the files used in model building and restraint generation.

\code{.py}
#---------------------------
# Define Input Files
#---------------------------
datadirectory = "../data/"
topology_file = datadirectory+"topology.txt"
target_gmm_file = datadirectory+'emd_1883.map.mrc.gmm.50.txt'
\endcode

The first section defines where input files are located.  The
[topology file](https://github.com/salilab/imp_tutorial/blob/pmi2/rnapolii/data/topology.txt)
defines how the system components are structurally represented. `target_gmm_file` stores the EM map for the entire complex, which has already been converted into a Gaussian mixture model.

**Build the Model Representation Using a Topology File**
Using the topology file we define the overall topology: we introduce the
molecules with their sequence and their known structure, and define the movers.
Each line in the file is a user-defined molecular **Domain**, and each column
contains the specifics needed to build the system.
See the [TopologyReader](@ref IMP::pmi::topology::TopologyReader) documentation
for a full description of the topology file format.

\code{.py}
# Initialize model
m = IMP.Model()

# Read in the topology file.
# Specify the directory wheere the PDB files, fasta files and GMM files are
topology = IMP.pmi.topology.TopologyReader(topology_file,
                                  pdb_dir=datadirectory,
                                  fasta_dir=datadirectory,
                                  gmm_dir=datadirectory)

# Use the BuildSystem macro to build states from the topology file
bs = IMP.pmi.macros.BuildSystem(m)

# Each state can be specified by a topology file.
bs.add_state(topology)
\endcode

**Building the System Representation and Degrees of Freedom**

Here we can set the **Degrees of Freedom** parameters, which should be
optimized according to MC acceptance ratios. There are three kind of movers:
Rigid Body, Bead, and Super Rigid Body (super rigid bodies are sets of
rigid bodies and beads that will move together in an additional Monte Carlo
move). 

`max_rb_trans` and `max_rb_rot` are the maximum translation and rotation
of the Rigid Body mover, `max_srb_trans` and `max_srb_rot` are the maximum
translation and rotation of the Super Rigid Body mover and `max_bead_trans`
is the maximum translation of the Bead Mover.

The excecution of the macro will return the root hierarchy (`root_hier`)
and the degrees of freedom (`dof`) objects, both of which are used later on.

\code{.py}
root_hier, dof = bs.execute_macro(max_rb_trans=4.0,
                                  max_rb_rot=0.3,
                                  max_bead_trans=4.0,
                                  max_srb_trans=4.0,
                                  max_srb_rot=0.3)
\endcode

Since we're interested in modeling the stalk, we will fix all subunits
except Rpb4 and Rpb7. Note that we are using IMP.atom.Selection to get the
particles that correspond to the fixed Molecules.

\code{.py}
# Fix all rigid bodies but not Rpb4 and Rpb7 (the stalk)
# First select and gather all particles to fix.
fixed_particles=[]
for prot in ["Rpb1","Rpb2","Rpb3","Rpb5","Rpb6","Rpb8","Rpb9","Rpb10","Rpb11","Rpb12"]:
    fixed_particles+=IMP.atom.Selection(root_hier,molecule=prot).get_selected_particles()

# Fix the Corresponding Rigid movers and Super Rigid Body movers using dof
# The flexible beads will still be flexible (fixed_beads is an empty list)!
fixed_beads,fixed_rbs=dof.disable_movers(fixed_particles,
                                         [IMP.core.RigidBodyMover,
                                          IMP.pmi.TransformMover])
\endcode

Finally we randomize the initial configuration to remove any bias from the
initial starting configuration read from input files. Since each subunit is
composed of rigid bodies (i.e., beads constrained in a structure) and flexible
beads, the configuration of the system is initialized by displacing each
mobile rigid body and each bead randomly by 50 Angstroms, and rotate them
randomly, and far enough from each other to prevent any steric clashes. 

The `excluded_rigid_bodies=fixed_rbs` will exclude from the randomization
everything that was fixed above.

\code{.py}
# Randomize the initial configuration before sampling, of only the molecules
# we are interested in (Rpb4 and Rpb7)
IMP.pmi.tools.shuffle_configuration(root_hier,
                                    excluded_rigid_bodies=fixed_rbs,
                                    max_translation=50,
                                    verbose=False,
                                    cutoff=5.0,
                                    niterations=100)
\endcode

### Set up Restraints

After defining the representation of the model, we build the restraints by which the individual structural models will be scored based on the input data.

**Connectivity Restraint**
\code{.py}
# Connectivity keeps things connected along the backbone (ignores if inside
# same rigid body)
mols = IMP.pmi.tools.get_molecules(root_hier)
for mol in mols:
    molname=mol.get_name()
    IMP.pmi.tools.display_bonds(mol)
    cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol,scale=2.0)
    cr.add_to_model()
    cr.set_label(molname)
    outputobjects.append(cr)
\endcode

**Excluded Volume Restraint**
\code{.py}
ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
                                         included_objects=root_hier,
                                         resolution=10)
ev.add_to_model()
outputobjects.append(ev)
\endcode

The excluded volume restraint is calculated at resolution 10 (20 residues per bead).


**Crosslinks**

A crosslinking restraint is implemented as a distance restraint between two residues.  The two residues are each defined by the protein (component) name and the residue number.  The script here extracts the correct four columns that provide this information from the [input data file](@ref rnapolii_1).

\code{.py}
xldbkwc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
xldbkwc.set_protein1_key("pep1.accession")
xldbkwc.set_protein2_key("pep2.accession")
xldbkwc.set_residue1_key("pep1.xlinked_aa")
xldbkwc.set_residue2_key("pep2.xlinked_aa")

xl1db = IMP.pmi.io.crosslink.CrossLinkDataBase(xldbkwc)
xl1db.create_set_from_file(datadirectory+'polii_xlinks.csv')

xl1 = IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint(
                                   root_hier=root_hier,
                                   CrossLinkDataBase=xl1db,
                                   length=21.0,
                                   slope=0.01,
                                   resolution=1.0,
                                   label="Trnka",
                                   weight=1.)

xl1.add_to_model()
\endcode

An object `xl1` for this crosslinking restraint is created and then added to the model.
* `length`: The maximum length of the crosslink
* `slope`: Slope of linear energy function added to sigmoidal restraint
* `resolution`: The resolution at which the restraint is evaluated. 1 = residue level
* `label`: A label for this set of cross links - helpful to identify them later in the stat file

**EM Restraint**

\code{.py}
em_components = IMP.pmi.tools.get_densities(root_hier)

gemt = IMP.pmi.restraints.em.GaussianEMRestraint(em_components,
                                                 target_gmm_file,
                                                 scale_target_to_mass=True,
                                                 slope=0.000001,
                                                 weight=80.0)
gemt.add_to_model()
outputobjects.append(gemt)
\endcode

The GaussianEMRestraint uses a density overlap function to compare model to data. First the EM map is approximated with a Gaussian Mixture Model (done separately). Second, the components of the model are represented with Gaussians (forming the model GMM)
* `scale_to_target_mass` ensures the total mass of model and map are identical
* `slope`: nudge model closer to map when far away
* `weight`: heuristic, needed to calibrate the EM restraint with the other terms. 

and then add it to the output object.

---

Completion of these steps sets the energy function.
The next step is \ref rnapolii_3.
