## \example pmi/symmetry.py
"""Clone molecules and use a symmetry constrant
"""

import IMP
import RMF
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints.stereochemistry
import math

# Create System and State
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st = s.create_state()

# Create a simple all-bead molecule
mol = st.create_molecule("mymol",sequence='A'*10,chain_id='A')
mol.add_representation(mol,
                       resolutions=[1])

# Clone the molecule multiple times
# Calling molecule.create_clone makes a new molecule with the same name, sequence,
#  initial structure, and choice of representations
# Note: another function, molecule.create_copy(), just copies the name and sequence
mols = [mol]
chains='BCDEFGHI'
for nc in range(7):
    clone = mol.create_clone(chains[nc])
    mols.append(clone)

hier = s.build()

# Create a symmetry constraint
#  A constrant is invariant: IMP will automatically move all clones to match the reference
#  If instead you want some more flexiblity, consider IMP.pmi.restraints.stereochemistry.SymmetryRestraint
dof = IMP.pmi.dof.DegreesOfFreedom(mdl)
center = IMP.algebra.Vector3D([50,0,0])
for nc in range(7):
    rot = IMP.algebra.get_rotation_about_axis([0,0,1],2*math.pi*(nc+1)/8)
    transform = IMP.algebra.get_rotation_about_point(center,rot)
    dof.constrain_symmetry(mols[0],mols[nc+1],transform)
mdl.update() # propagates coordinates


############ Make stuff look cool with restraints ###########

# set up the original molecule as flexible beads
dof.create_flexible_beads(mols[0])

# Create a connectivity restraint for the first molecule
cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(objects=mol)
cr.add_to_model()

# Create excluded volume for all particles
evr = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(included_objects=mols)
evr.add_to_model()

# Quickly move all flexible beads into place
dof.optimize_flexible_beads(100)

# write a single-frame RMF to view the helix
out = IMP.pmi.output.Output()
out.init_rmf("example_symmetry.rmf3",hierarchies=[hier])
out.write_rmf("example_symmetry.rmf3")
