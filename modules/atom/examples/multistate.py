## \example multistate.py
#

import IMP.atom
import IMP.kernel

m = IMP.kernel.Model()

rt = IMP.atom.Hierarchy.setup_particle(m, m.add_particle("root"))


def create_one():
    h = IMP.atom.read_pdb(IMP.atom.get_example_path("1d3d-protein.pdb"), m)
    return h

h0 = create_one()
rt.add_child(h0)
IMP.atom.State.setup_particle(h0, 0)
h1 = create_one()
rt.add_child(h1)
IMP.atom.State.setup_particle(h1, 1)


r8 = IMP.atom.Selection(
    tr,
    state=1,
    residue_index=8,
    atom_type=IMP.atom.AT_CA)

# we get the 8th CA from state 1
print r8
