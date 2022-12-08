## \example pmi/membrane.py
"""
This script shows how to simulate an a protein using a
Membrane restraint. This example show 3 ideal helices
that have different segments inside the membrane
"""

import IMP
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints
import IMP.pmi.restraints.basic
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.basic
import sys

IMP.setup_from_argv(sys.argv,
                    "Simulation of a protein using a membrane restraint")

model = IMP.Model()
s = IMP.pmi.topology.System(model)
st = s.create_state()

# Create molecules and add helix representation
# this makes a c-alpha chain in approximately the shape of a helix
mols = []

for i, len_helix in enumerate([20, 40, 80]):
    mol = st.create_molecule(
        "helix_%s" % (str(i)), sequence="A" * len_helix, chain_id="A"
    )
    mol.add_representation(mol, resolutions=[1], ideal_helix=True)
    mols.append(mol)
hier = s.build()

dof = IMP.pmi.dof.DegreesOfFreedom(model)
for mol in mols:
    dof.create_rigid_body(mol,
                          max_trans=3.0,
                          max_rot=0.5)
##############################
# Connectivity
##############################
output_objects = []
sample_objects = []
rmf_restraints = []

crs = []
for mol in mols:
    cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(mol)
    cr.set_label(mol.get_name())
    cr.add_to_model()
    output_objects.append(cr)
    crs.append(cr)

##############################
# Excluded Volume
##############################
evr1 = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
    included_objects=mols, resolution=1
)
evr1.add_to_model()
evr1.set_weight(1.0)
output_objects.append(evr1)

##############################
# Membrane restraint
##############################

mr = IMP.pmi.restraints.basic.MembraneRestraint(
    hier,
    objects_inside=["helix_0", (1, 20, "helix_1"), (20, 40, "helix_2")],
    objects_above=[(21, 40, "helix_1"), (1, 19, "helix_2")],
    objects_below=[(41, 80, "helix_2")],
    weight=10,
)

mr.add_to_model()
output_objects.append(mr)

# For visualization purposes
mr.create_membrane_density()

###########################
# External barrier
###########################
eb = IMP.pmi.restraints.basic.ExternalBarrier(hierarchies=hier, radius=50)
eb.add_to_model()

##############################
# Shuffle
##############################
IMP.pmi.tools.shuffle_configuration(hier, max_translation=300)
print(dof.get_movers())

##############################
# Sampling
##############################

frames = 10 if IMP.get_is_quick_test() else 2000
rex = IMP.pmi.macros.ReplicaExchange(
    model, root_hier=hier, monte_carlo_sample_objects=dof.get_movers(),
    replica_exchange_maximum_temperature=3.0,
    global_output_directory="output/", output_objects=output_objects,
    monte_carlo_steps=10, number_of_frames=frames,
    number_of_best_scoring_models=0)

rex.execute_macro()
exit()
