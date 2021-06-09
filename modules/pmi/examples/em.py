## \example pmi/em.py
"""This script shows how to create DENSITY representations.
and set up the Gaussian EM restraint.

These representations are useful when you are doing EM fitting with
rigid bodies.

Preliminary step: you should convert your EM map to a GMM file
with the command line utility create_gmm.py
(located in isd/pyext/src/create_gmm.py):
python create_gmm.py emd_1883.mrc 50 emd_1883.gmm50.txt -m emd_1883.gmm50.mrc
"""

import IMP
import IMP.atom
import IMP.algebra
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints.em
import sys

IMP.setup_from_argv(sys.argv, "Set up the EM restraint")

# ##################### SYSTEM SETUP #####################
# Preliminaries
mdl = IMP.Model()
seqs = IMP.pmi.topology.Sequences(IMP.pmi.get_example_path('data/1WCM.fasta'))

# Setup just one molecule
s = IMP.pmi.topology.System(mdl)
st = s.create_state()
mol = st.create_molecule("Rpn4", sequence=seqs["1WCM:D"], chain_id="D")
atomic_res = mol.add_structure(
    IMP.pmi.get_example_path('data/1WCM_fitted.pdb'), chain_id="D", offset=0)

# Below we create a GMM approximation for this moleucle
# This "DENSITY" representation is used in the GaussianEMRestraint (and others
# in the future)
# For structure regions we "fit" GMM components to all atom centers
mol.add_representation(
    atomic_res,
    resolutions=[1, 10],
    # how much to coarsen this representation
    density_residues_per_component=10,
    # will write a .txt and .mrc file forcomponent
    density_prefix="Rpn4_gmm",
    # set True if you want to overwrite
    density_force_compute=False,
    # set to 0 if you don't care about writing the map
    # if rasterizing takes too long, increase this value
    density_voxel_size=3.0)

# for the unstructured regions, we simply decorate each bead AS a Gaussian,
# so no fitting is necessary
mol.add_representation(mol.get_non_atomic_residues(),
                       resolutions=[10],
                       # just set this flag, nothing is written
                       setup_particles_as_densities=True)

hier = s.build()

# You can always check the representations with:
IMP.atom.show_with_representations(hier)

# ##################### RESTRAINTS #####################
output_objects = []

# To add the GaussianEMRestraint, first select all densities
densities = IMP.atom.Selection(
    hier, representation_type=IMP.atom.DENSITIES).get_selected_particles()
emr = IMP.pmi.restraints.em.GaussianEMRestraint(
    densities,
    # created by user, see top of file
    target_fn=IMP.pmi.get_example_path('data/emd_1883.gmm50.txt'),
    # a small number, helps drag bits into map
    slope=0.01,
    # if the model is the same size as map, usually set to True
    scale_target_to_mass=False,
    # manually set the mass of the target map (remove if you set above to True)
    target_mass_scale=100000,
    # the data weight
    weight=100.0)
emr.add_to_model()
output_objects.append(emr)
mdl.update()
print(emr.evaluate())
