## \example pmi/automatic.py
"""This script shows how to use the BuildSystem macro to construct large systems with minimal code
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
import tempfile,os

# This is the topology table format.
#  It allows you to create many components in a simple way
#  By default it sets up each molecule as a rigid body, filling in missing pieces with beads
#  You can create multiple domains by specifying different segments, as in Rpb1 below
#  You can also create copies of molecules by appending '.X' to the name, as in Rpb4 below
topology='''
|molecule_name|color|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rigid_body|super_rigid_body|chain_of_super_rigid_bodies|
|Rpb1  |blue      |1WCM.fasta|1WCM:A|1WCM_fitted.pdb|A|1,100  |0 |5|0 |1|1,2| |
|Rpb1  |cyan      |1WCM.fasta|1WCM:A|BEADS          |A|101,150|0 |5|0 |2|1,2| |
|Rpb2  |red       |1WCM.fasta|1WCM:B|1WCM_fitted.pdb|B|1,END  |0 |5|0 |3|1  | |
|Rpb3  |green     |1WCM.fasta|1WCM:C|1WCM_fitted.pdb|C|1,END  |0 |5|0 |4|1  | |
|Rpb4  |orange    |1WCM.fasta|1WCM:D|1WCM_fitted.pdb|D|1,END  |0 |5|0 |5|1,3| |
|Rpb4.1|yellow    |1WCM.fasta|1WCM:D|1WCM_fitted.pdb|D|1,END  |0 |5|0 |6|1,3| |
|Rpb4.2|salmon    |1WCM.fasta|1WCM:D|1WCM_fitted.pdb|D|1,END  |0 |5|0 |7|1,3| |
|Rpb5  |gold      |1WCM.fasta|1WCM:E|BEADS          | |1,50   |  |5|0 |8|1  | |
|Rpb5  |pink      |1WCM.fasta|1WCM:E|IDEAL_HELIX    | |51,100 |  |5|0 |8|1  | |
'''

# Normally the topology table is kept in a text file but here we just write it to a temporary one
tf = tempfile.NamedTemporaryFile(delete=False)
tf.write(topology)
tf.close()

# The TopologyReader reads the text file, and the BuildSystem macro constructs it
mdl = IMP.Model()
reader = IMP.pmi.topology.TopologyReader(tf.name,
                                         pdb_dir = IMP.pmi.get_example_path('data/'),
                                         fasta_dir = IMP.pmi.get_example_path('data/'),
                                         gmm_dir = IMP.pmi.get_example_path('data/'))
bs = IMP.pmi.macros.BuildSystem(mdl)
bs.add_state(reader) # note you can call this multiple times to create a multi-state system
hier, dof = bs.execute_macro()

###################### RESTRAINTS #####################
output_objects = [] # keep a list of functions that need to be reported

# Connectivity keeps things connected along the backbone (ignores if inside same rigid body)
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
evr = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects = mols)
evr.add_to_model()
output_objects.append(evr)


###################### SAMPLING #####################


# mix it up so it looks cool
IMP.pmi.tools.shuffle_configuration(hier)

# Quickly move all flexible beads into place
dof.optimize_flexible_beads(100)

rex=IMP.pmi.macros.ReplicaExchange0(mdl,
                                    root_hier=hier,
                                    monte_carlo_sample_objects=dof.get_movers(),
                                    global_output_directory='auto_output/',
                                    output_objects=output_objects,
                                    monte_carlo_steps=10,
                                    number_of_best_scoring_models=0,
                                    number_of_frames=100)
rex.execute_macro()



os.remove(tf.name)
