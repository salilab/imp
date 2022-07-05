## \example pmi/atomistic.py
"""This script shows how to simulate an atomic system with MD,
with a secondary structure elastic network to speed things up.
"""

from __future__ import print_function
import IMP
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints.stereochemistry
import sys

IMP.setup_from_argv(sys.argv, "Simulation of an atomic system")
if IMP.get_is_quick_test():
    print("This example is too slow to test in debug mode - run without")
    print("internal tests enabled, or without the --run-quick-test flag")
    sys.exit(0)

# Setup System and add a State
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st1 = s.create_state()

# Read sequences and create Molecules
seqs = IMP.pmi.topology.Sequences(IMP.pmi.get_example_path('data/gcp2.fasta'))
gcp2 = st1.create_molecule("GCP2", sequence=seqs["GCP2_YEAST"], chain_id='A')

# Add structure. This function returns a list of the residues that now
# have structure
a1 = gcp2.add_structure(IMP.pmi.get_example_path('data/gcp2.pdb'),
                        chain_id='A')

# Add structured part representation and then build
gcp2.add_representation(a1, resolutions=[0])
print('building molecule')
hier = s.build()

# add charmm restraints
print('adding restraints')
charmm = IMP.pmi.restraints.stereochemistry.CharmmForceFieldRestraint(hier)
charmm.add_to_model()

# add elastic network on secondary structure units
sses = IMP.pmi.io.parse_dssp(IMP.pmi.get_example_path('data/gcp2.dssp'), 'A',
                             name_map={'A': 'GCP2'})
all_rs = []
for sse in sses['helix']+sses['beta']:
    er = IMP.pmi.restraints.stereochemistry.ElasticNetworkRestraint(
        selection_tuples=sse,
        strength=10.0,
        dist_cutoff=5.0,
        ca_only=True,
        hierarchy=hier)
    all_rs.append(er)
    er.add_to_model()

# setup MD and run
dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
md_ps = dof.setup_md(gcp2)
rex = IMP.pmi.macros.ReplicaExchange0(
    mdl,
    root_hier=hier,
    molecular_dynamics_sample_objects=md_ps,
    molecular_dynamics_steps=5,
    # set >0 to store best PDB files (but this is slow to do online)
    number_of_best_scoring_models=0,
    # increase number of frames to get better results!
    number_of_frames=1,
    global_output_directory='atomistic_output/')
rex.execute_macro()
