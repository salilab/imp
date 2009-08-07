tr= IMP.core.HierarchyTraits("my hierarchy")
pd= IMP.core.Hierarchy.setup_particle(parent_particle, tr)
for p in children_particles:
    cd= IMP.core.Hierarchy.setup_particle(p, tr)
    pd.add_child(cd)
pd.show()
