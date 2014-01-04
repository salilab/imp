## \example multistate.py
#

import IMP.atom
import IMP.kernel

m = IMP.kernel.Model()


def create_one():
    h = IMP.atom.read_pdb(IMP.atom.get_example_path("1d3d-protein.pdb"), m)
    return h

h0 = create_one()
s = IMP.atom.State.setup_particle(h0)
h1 = create_one()
# the substate needs to have the same tree structure as the tree rooted at h0
s.add_state(h1)

r8 = IMP.atom.Selection(
    h0,
    state=IMP.atom.ALL_STATES,
    residue_index=8,
    atom_type=IMP.atom.AT_CA)

# we get the 8th CA from all states
print r8
