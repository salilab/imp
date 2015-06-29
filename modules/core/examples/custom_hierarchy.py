## \example core/custom_hierarchy.py
# This example shows how to create and use a custom hierarchy of particles.

import IMP
import IMP.core
import sys

IMP.setup_from_argv(sys.argv, "custom hierarchy")

def custom_hierarchy(parent_particle, children_particles):
    tr = IMP.core.HierarchyTraits("my hierarchy")
    pd = IMP.core.Hierarchy.setup_particle(parent_particle, tr)
    for p in children_particles:
        cd = IMP.core.Hierarchy.setup_particle(p, tr)
        pd.add_child(cd)
    pd.show()

m = IMP.Model()
p1 = IMP.Particle(m)
p2 = IMP.Particle(m)
p3 = IMP.Particle(m)
custom_hierarchy(p1, (p2, p3))
