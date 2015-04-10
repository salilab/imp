## \example multistate.py
# Show how to create an detect files where frames represent multiple
# states of the same thing.

from __future__ import print_function
import RMF

tfn = RMF._get_temporary_file_path("multistate.rmf3")
print("File is", tfn)

fh = RMF.create_rmf_file(tfn)
rh = fh.get_root_node()
pf = RMF.ParticleFactory(fh)
bf = RMF.BondFactory(fh)
sf = RMF.StateFactory(fh)


def create_hierarchy(state):
    ns = []
    rt = rh.add_child("state" + str(state), RMF.REPRESENTATION)
    sf.get(rt).set_static_state_index(state)
    for i in range(0, 3):
        n = rt.add_child(str(i), RMF.REPRESENTATION)
        ns.append(n)
        pf.get(n).set_static_mass(1)
        pf.get(n).set_static_radius(1)
    for i in range(0, 2):
        b = rt.add_child("b", RMF.BOND)
        bd = bf.get(b)
        bd.set_bonded_0(ns[i].get_id().get_index())
        bd.set_bonded_1(ns[i + 1].get_id().get_index())
    return ns

all_ps = []
for i in range(0, 3):
    all_ps.append(create_hierarchy(i))

for frame in range(0, 4):
    fh.add_frame(str(i), RMF.FRAME)
    for state, sps in enumerate(all_ps):
        for particle, p in enumerate(sps):
            pf.get(
                p).set_coordinates(RMF.Vector3(particle + frame + .1 * state,
                                               frame + .1 * state,
                                               frame + .1 * state))
