## \example pmi/protein_residue_binding.py
"""
This script shows how to simulate residue–protein 
binding contacts inferred from mutagenesis studies.
This example shows protein A binding to protein B 
through a set of residues predicted to be required 
for binding in mutagensis studies.
"""

from __future__ import print_function
import IMP
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.residue_proximity_restraint
import tempfile
import os
import sys

topology = '''
|molecule_name|color|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rigid_body|super_rigid_body|chain_of_super_rigid_bodies|
|Rpb4  |red   |1WCM.fasta |1WCM:D   |1WCM_fitted.pdb    |D|1,END  |0 |5|0 |1 | | ||
|Rpb7  |gold  |1WCM.fasta |1WCM:G   |1WCM_fitted.pdb    |G|1,END  |0 |5|0 |2 | | ||
'''

# Normally the topology table is kept in a text file but here we just write
# it to a temporary one
tf = tempfile.NamedTemporaryFile(delete=False, mode='w')
tf.write(topology)
tf.close()

print(IMP.pmi.get_example_path('data/'))

# The TopologyReader reads the text file, and the BuildSystem macro
# constructs it
mdl = IMP.Model()
reader = IMP.pmi.topology.TopologyReader(
    tf.name, pdb_dir=IMP.pmi.get_example_path('data/'),
    fasta_dir=IMP.pmi.get_example_path('data/'),
    gmm_dir=IMP.pmi.get_example_path('data/'))
bs = IMP.pmi.macros.BuildSystem(mdl)
# note you can call this multiple times to create a multi-state system
bs.add_state(reader)
hier, dof = bs.execute_macro()

# ################ STEREOCHEMISTRY RESTRAINTS ################

output_objects = []  # keep a list of functions that need to be reported

# Connectivity keeps things connected along the backbone (ignores if inside
# same rigid body)
crs = []
moldict = bs.get_molecules()[0]
mols = []
for molname in moldict:
    for mol in moldict[molname]:
        cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
        cr.add_to_model()
        output_objects.append(cr)
        crs.append(cr)
        mols.append(mol)

# Excluded volume - automatically more efficient due to rigid bodies
evr = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
    included_objects=mols)
evr.add_to_model()
output_objects.append(evr)

# External barrier- Avoid proteins to drift away
eb = IMP.pmi.restraints.basic.ExternalBarrier(hierarchies=hier, radius=500)
eb.add_to_model()

# ################# PROTEIN-RESIDUE PROXIMITY ################

br = IMP.pmi.restraints.residue_proximity_restraint.ResidueProteinProximityRestraint(
    hier,
    selection  = ('Rpb7',38, 44, 'Rpb4'),
    label = 'B38_44') 

br.add_to_model()
br.set_weight(5.0)
output_objects.append(br)
br.get_output()

# ##################### SAMPLING #######################

# Fix rigid-body 

part_p1=IMP.atom.Selection(hier,
                           molecule="Rpb4").get_selected_particles()

xyzs, rbs=dof.disable_movers(part_p1,
                                mover_types=[IMP.core.RigidBodyMover])


# mix it up so it looks cool
IMP.pmi.tools.shuffle_configuration(hier)

# Quickly move all flexible beads into place
dof.optimize_flexible_beads(100)

rex = IMP.pmi.macros.ReplicaExchange0(
    mdl,
    root_hier=hier,
    monte_carlo_sample_objects=dof.get_movers(),
    global_output_directory='output/',
    output_objects=output_objects,
    monte_carlo_steps=10,
    number_of_best_scoring_models=0,
    number_of_frames=5000)
rex.execute_macro()

os.remove(tf.name)
