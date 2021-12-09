## \example pmi/ideal_helix.py
"""Short example demonstrating ideal helices in PMI
"""

import IMP
import IMP.atom
import IMP.rmf
import IMP.pmi
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.macros
import IMP.pmi.restraints
import sys

IMP.setup_from_argv(sys.argv, "Demonstration of ideal helices")

# Create System and State
mdl = IMP.Model()
s = IMP.pmi.topology.System(mdl)
st = s.create_state()

# Create a molecule and add helix representation
#  this makes a c-alpha chain in approximately the shape of a helix
mol = st.create_molecule("example_helix", sequence='A'*20, chain_id='A')
mol.add_representation(mol,
                       resolutions=[1, 10],
                       ideal_helix=True)
hier = s.build()

# write a single-frame RMF to view the helix
out = IMP.pmi.output.Output()
out.init_rmf("example_helix.rmf3", hierarchies=[hier])
out.write_rmf("example_helix.rmf3")
