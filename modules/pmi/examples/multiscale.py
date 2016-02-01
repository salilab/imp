## \example pmi/multiscale.py
"""This script shows how to represent a system at multiple scales and do basic sampling.
"""

import IMP
import RMF
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints
import IMP.pmi.restraints.stereochemistry

###################### SYSTEM SETUP #####################
# Read sequences etc
seqs = IMP.pmi.topology.Sequences('data/1WCM.fasta')
components = ["Rpb1","Rpb2","Rpb3","Rpb4"]
chains = "ABCD"
beadsize = 10

# Setup System and add a State
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st = s.create_state()

# Add Molecules for each component as well as representations
mols = []
for n in range(len(components)):
    print('PMI: setting up',components[n],'1WCM:'+chains[n])
    mol = st.create_molecule(                      # create molecule
        components[n],
        sequence=seqs['1WCM:'+chains[n]],
        chain_id=chains[n])
    atomic = mol.add_structure('data/1WCM_fitted.pdb',            # read in structure data
                               chain_id=chains[n],
                               offset=0)
    mol.add_representation(atomic,                 # res 1,10 for structured regions
                           resolutions=[1,10])
    mol.add_representation(mol[:]-atomic,          # res 10 for unstructured regions
                           resolutions=[beadsize])
    mols.append(mol)

# calling System.build() creates all States and Molecules (and their representations)
#  Once you call build(), anything without representation is destroyed.
#  You can still use handles like molecule[a:b], molecule.get_atomic_residues() or molecule.get_non_atomic_residues()
#  However these functions will only return BUILT representations
root_hier = s.build()

# Setup degrees of freedom
#  The DOF functions automatically select all resolutions
#  Objects passed to nonrigid_parts move with the frame but also have their own independent mover.
dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
for mol in mols:
    dof.create_rigid_body(mol,
                          nonrigid_parts=mol.get_non_atomic_residues())


###################### RESTRAINTS #####################
output_objects = [] # keep a list of functions that need to be reported

# Connectivity keeps things connected along the backbone (ignores if inside same rigid body)
crs = []
for mol in mols:
    cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
    cr.add_to_model()
    output_objects.append(cr)
    crs.append(cr)

# Excluded volume - automatically more efficient due to rigid bodies
evr = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects = mols)
evr.add_to_model()
output_objects.append(evr)


###################### SAMPLING #####################
# Run replica exchange Monte Carlo sampling
rex=IMP.pmi.macros.ReplicaExchange0(mdl,
                                    root_hier=root_hier,                          # pass the root hierarchy
                                    crosslink_restraints=crs,                     # will display like XLs
                                    monte_carlo_sample_objects=dof.get_movers(),  # pass MC movers
                                    global_output_directory='multiscale_output/',
                                    output_objects=output_objects,
                                    monte_carlo_steps=10,
                                    number_of_frames=100)
rex.execute_macro()
