tr= IMP.core.HierarchyTraits("my hierarchy")
pd= IMP.core.HierarchyDecorator.create(parent_particle, tr)
for p in children_particles:
    cd= IMP.core.HierarchyDecorator.create(p, tr)
    pd.add_child(cd)
pd.show()
