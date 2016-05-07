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
import tempfile,shutil

# This is the topology table format.
#  It allows you to create many components in a simple way
#  By default it sets up each molecule as a rigid body, filling in missing pieces with beads
#  You can create multiple domains by specifying different segments, as in Rpb1 below
#  You can also create copies of molecules by appending '.X' to the name, as in Rpb4 below
topology='''
|molecule_name|color|fasta_fn|fasta_id|pdb_fn|chain|residue_range|pdb_offset|bead_size|em_residues_per_gaussian|rigid_body|super_rigid_body|chain_of_super_rigid_bodies|
|Rpb1  |blue   |1WCM.fasta|1WCM:A|1WCM_fitted.pdb|A|1,100  |0 |5|0 |1|1,2| |
|Rpb1  |cyan   |1WCM.fasta|1WCM:A|1WCM_fitted.pdb|A|101,200|0 |5|0 |2|1,2| |
|Rpb2  |red    |1WCM.fasta|1WCM:B|1WCM_fitted.pdb|B|1,-1   |0 |5|0 |3|1  | |
|Rpb3  |green  |1WCM.fasta|1WCM:C|1WCM_fitted.pdb|C|1,-1   |0 |5|0 |4|1  | |
|Rpb4  |orange |1WCM.fasta|1WCM:D|1WCM_fitted.pdb|D|1,-1   |0 |5|0 |5|1,3| |
|Rpb4.1|yellow |1WCM.fasta|1WCM:D|1WCM_fitted.pdb|D|1,-1   |0 |5|0 |6|1,3| |
|Rpb4.2|tomato |1WCM.fasta|1WCM:D|1WCM_fitted.pdb|D|1,-1   |0 |5|0 |7|1,3| |
|Rpb5  |dark green|1WCM.fasta|1WCM:E|BEADS       | |       |  |5|0 |8|1  | |
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

# write RMF
out = IMP.pmi.output.Output()
out.init_rmf("example_auto.rmf3",hierarchies=[hier])
out.write_rmf("example_auto.rmf3")

shutil.rmtree(tf.name)
