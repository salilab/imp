import IMP.atom
import IMP.kernel
m = IMP.kernel.Model()
all = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
all.set_name("the universe")
h = IMP.atom.create_protein(m, "ProteinA", 300., [0, 820, 1065, 2075])
all.add_child(h)
IMP.atom.show(h)
s0 = IMP.atom.Selection(hierarchy=all, molecule="ProteinA",
                        residue_indexes=range(0, 820)).get_selected_particles()
s1 = IMP.atom.Selection(hierarchy=all, molecule="ProteinA",
                        residue_indexes=range(820, 1065)).get_selected_particles()
print s0, [h.get_child(0)]
assert(s0 == [h.get_child(0)])
print s1, [h.get_child(1)]
assert(s1 == [h.get_child(1)])
