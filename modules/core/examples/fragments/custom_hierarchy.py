tr= IMP.core.HierarchyTraits("my hierarchy")
pd= IMP.core.Hierarchy.create(parent_particle, tr)
for p in children_particles:
    cd= IMP.core.Hierarchy.create(p, tr)
    pd.add_child(cd)
pd.show()
