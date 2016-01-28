## \example pmi/multiscale.py

"""This script shows how to represent a system
At multiple scales, including electron densities.
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
import IMP.pmi.restraints.em

# Setup System and add a State
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st1 = s.create_state()

# Read sequences and create Molecules
seqs = IMP.pmi.topology.Sequences('data/gcp2.fasta')
gcp2 = st1.create_molecule("GCP2",sequence=seqs["GCP2_YEAST"],chain_id='A')

# Add structure. This function returns a list of the residues that now have structure
a1 = gcp2.add_structure('data/gcp2.pdb',
                        chain_id='A')

# Add representations. For structured regions, created a few beads as well as densities
#  For unstructured regions, create a single bead level and set those up as densities
gcp2.add_representation(a1,
                        resolutions=[10,100],
                        density_prefix='data/gcp2_gmm',
                        density_residues_per_component=20,
                        density_voxel_size=3.0)
gcp2.add_representation(gcp2.get_non_atomic_residues(),
                        resolutions=[10],
                        setup_particles_as_densities=True)

# When you call build, this actually makes the beads and fits the GMMs
#  This returns a canonical IMP hierarchy
hier = s.build()
IMP.atom.show_molecular_hierarchy(hier)

# Setup degrees of freedom
#  This automatically selects all representations including densities
dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
dof.create_rigid_body(gcp2,
                      nonrigid_parts=gcp2.get_non_atomic_residues())

# Setup Gaussian EM restraint
density_ps = IMP.atom.Selection(gcp2.get_hierarchy(),representation_type = IMP.atom.DENSITIES).get_selected_particles() + \
             [h.get_hierarchy() for h in gcp2.get_non_atomic_residues()]
gem = IMP.pmi.restraints.em.GaussianEMRestraint(density_ps,
                                                'data/gcp2_gmm.txt')
gem.add_to_model()

rex=IMP.pmi.macros.ReplicaExchange0(mdl,
                                    root_hier=st1.get_hierarchy(),
                                    monte_carlo_sample_objects=dof.get_movers(),
                                    global_output_directory='gcp2_multiscale/',
                                    number_of_frames=10,
                                    monte_carlo_steps=10,
                                    test_mode=True)
rex.execute_macro()
