## \example restrainer/simple_em_fit.py
## This example shows how to create simple IMP::em::FitRestraint.
##

#-- File: simple_em_fit.py --#

import IMP
import IMP.atom
import IMP.restrainer

imp_model = IMP.Model()

# Create particle point 1
p1 = IMP.Particle(imp_model)
p1.add_attribute(IMP.FloatKey("x"), 12, True)
p1.add_attribute(IMP.FloatKey("y"), 12, True)
p1.add_attribute(IMP.FloatKey("z"), 12, True)
p1.add_attribute(IMP.FloatKey("radius"), 1.0)
p1.add_attribute(IMP.FloatKey("mass"), 1.0)
p1.add_attribute(IMP.FloatKey("protein"), 1.0)
p1.add_attribute(IMP.FloatKey("id"), 1.0)

# Create particle point 2
p2 = IMP.Particle(imp_model)
p2.add_attribute(IMP.FloatKey("x"), 15, True)
p2.add_attribute(IMP.FloatKey("y"), 6, True)
p2.add_attribute(IMP.FloatKey("z"), 6, True)
p2.add_attribute(IMP.FloatKey("radius"), 1.0)
p2.add_attribute(IMP.FloatKey("mass"), 1.0)
p2.add_attribute(IMP.FloatKey("protein"), 1.0)
p2.add_attribute(IMP.FloatKey("id"), 1.0)

# Create particle point 3
p3 = IMP.Particle(imp_model)
p3.add_attribute(IMP.FloatKey("x"), 6, True)
p3.add_attribute(IMP.FloatKey("y"), 15, True)
p3.add_attribute(IMP.FloatKey("z"), 15, True)
p3.add_attribute(IMP.FloatKey("radius"), 1.0)
p3.add_attribute(IMP.FloatKey("mass"), 1.0)
p3.add_attribute(IMP.FloatKey("protein"), 1.0)
p3.add_attribute(IMP.FloatKey("id"), 1.0)

mp = IMP.atom.Hierarchy.setup_particle(p1)
mp = IMP.atom.Hierarchy.setup_particle(p2)
mp = IMP.atom.Hierarchy.setup_particle(p3)

particles = []
particles.append(p1)
particles.append(p2)
particles.append(p3)

mhs = IMP.atom.Hierarchies()
mhs.append(mp)

dmap = IMP.restrainer.load_em_density_map (
           IMP.restrainer.get_example_path("in.mrc"), 1.0, 3.0)

se = IMP.restrainer.create_simple_em_fit(mhs, dmap)

r = se.get_restraint()

imp_model.add_restraint(r)
