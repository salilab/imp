## \example pmi/ambiguity.py
"""This script shows how to create a system with multiple copies of the same molecule.
We also create some crosslinks which take into account the ambiguity.
The key to ambiguity is using the same molecule name for ambiguous copies.
That way when you perform Selection it automatically finds all relevant molecules.
"""

import IMP
import RMF
import IMP.atom
import IMP.algebra
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.crosslinking
import tempfile
import os
###################### SYSTEM SETUP #####################

### Setup multistate system
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st1 = s.create_state()
st2 = s.create_state()

### For each state add some molecules - we'll make some bead only structures
# State 1: ProtA (chainA), ProtA (chainB), ProtC (chainC)
sequence = 'A'*10
m1A = st1.create_molecule('ProtA',sequence,chain_id='A')
m1A.add_representation(m1A,resolutions=[1])
m1B = m1A.create_clone(chain_id='B') # create_clone() will copy name/structure/representation
                                     # You cannot edit it!
                                     # There is also a function create_copy() which
                                     #  only copies the name, then you can change reps
m1C = st1.create_molecule('ProtC',sequence,chain_id='C')
m1C.add_representation(m1C,resolutions=[1])

# State 2: ProtA (chainA), ProtC (chainC)
m2A = st2.create_molecule('ProtA',sequence,chain_id='A')
m2A.add_representation(m2A,resolutions=[1])
m2C = st2.create_molecule('ProtC',sequence,chain_id='C')
m2C.add_representation(m2C,resolutions=[1])
root_hier = s.build()

### Display all the states, molecules, representations
IMP.atom.show_with_representations(root_hier)

### Setup all molecules to move as flexible beads and super rigid bodies
#  "Super rigid bodies" aren't really rigid, it's just a mover that moves the whole body together
dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
for mol in (m1A,m1B,m1C,m2A,m2C):
    dof.create_flexible_beads(mol,
                              max_trans=0.1)
    dof.create_super_rigid_body(mol)


###################### RESTRAINTS #####################
output_objects = [] # keep a list of functions that need to be reported
rmf_restraints = [] # display these restraints as springs in the RMF

### Crosslinks setup
# 1) Create file. This one XL has 3 ambiguity options: State1 has 2, State2 has 1
lines = '''id,mol1,res1,mol2,res2,score
1,ProtA,3,ProtC,9,1.0
'''
fd, filename = tempfile.mkstemp()
os.write(fd,lines)
os.close(fd)
tf = tempfile.NamedTemporaryFile

# 2) Define the columns
kw = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
kw.set_unique_id_key("id")
kw.set_protein1_key("mol1")
kw.set_protein2_key("mol2")
kw.set_residue1_key("res1")
kw.set_residue2_key("res2")
kw.set_id_score_key("score")
xldb = IMP.pmi.io.crosslink.CrossLinkDataBase(kw)
xldb.create_set_from_file(filename)
os.remove(filename)

# 3) Add the restraint
xlr = IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint(
    root_hier=root_hier,
    CrossLinkDataBase=xldb,
    length=21,
    label="XL",
    resolution=1,
    slope=0.01)
xlr.add_to_model()
output_objects.append(xlr)
rmf_restraints.append(xlr)

### Connectivity keeps things connected along the backbone
crs = []
for mol in (m1A,m1B,m1C,m2A,m2C):
    cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
    cr.add_to_model()
    output_objects.append(cr)
    rmf_restraints.append(cr)

### Excluded volume - one for each state (they don't interact)
evr1 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects = (m1A,m1B,m1C))
evr1.add_to_model()
output_objects.append(evr1)
evr2 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects = (m2A,m2C))
evr2.add_to_model()
output_objects.append(evr2)

###################### SAMPLING #####################
# randomize particles a bit
IMP.pmi.tools.shuffle_configuration(root_hier,
                                    max_translation=20)

# Shift state 2
# Even though the two states don't interact,
#  it'll be easier to look at the RMF if we separate them
trans = IMP.algebra.Transformation3D([50,0,0])
for fb in IMP.core.get_leaves(m2A.get_hierarchy())+IMP.core.get_leaves(m2C.get_hierarchy()):
    IMP.core.transform(IMP.core.XYZ(fb),trans)

# Run replica exchange Monte Carlo sampling
rex=IMP.pmi.macros.ReplicaExchange0(mdl,
                                    root_hier=root_hier,                          # pass the root hierarchy
                                    crosslink_restraints=rmf_restraints,          # will display as springs
                                    monte_carlo_sample_objects=dof.get_movers(),  # pass MC movers
                                    global_output_directory='ambiguity_output/',
                                    output_objects=output_objects,
                                    monte_carlo_steps=10,
                                    number_of_frames=1)     # increase number of frames to get better results!
rex.execute_macro()
