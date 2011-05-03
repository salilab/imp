#-- File: ms_connectivity_restraint.py --#

import IMP
import IMP.core
import IMP.algebra

# Setup model

m = IMP.Model()
ps= [IMP.Particle(m) for x in xrange(6)]
ds= []
ds.append(IMP.core.XYZ.setup_particle(ps[0], IMP.algebra.Vector3D(0.0, 0.0, 0.0)))
ds.append(IMP.core.XYZ.setup_particle(ps[1], IMP.algebra.Vector3D(1.0, 1.0, 0.0)))
ds.append(IMP.core.XYZ.setup_particle(ps[2], IMP.algebra.Vector3D(2.0, 0.0, 0.0)))
ds.append(IMP.core.XYZ.setup_particle(ps[3], IMP.algebra.Vector3D(3.0, 0.0, 0.0)))
ds.append(IMP.core.XYZ.setup_particle(ps[4], IMP.algebra.Vector3D(4.0, -1.0, 0.0)))
ds.append(IMP.core.XYZ.setup_particle(ps[5], IMP.algebra.Vector3D(1000, 1000, 1000)))

# Make sure that each component particle has an attribute called "id" as an identifier.

idk = IMP.StringKey("id");
ds[0].add_attribute(idk, "A")
ds[1].add_attribute(idk, "A")
ds[2].add_attribute(idk, "B")
ds[3].add_attribute(idk, "B")
ds[4].add_attribute(idk, "C")
ds[5].add_attribute(idk, "D")

# Create an experimental tree

T = IMP.core.ExperimentalTree()

# Add vertices to the tree
# add_node() returns a unique node handle that can be used as an argument to connect() later on.

i1 = T.add_node("A A B B C")
i2 = T.add_node("A B B C")
i3 = T.add_node("A A B B")
i4 = T.add_node("A B")
i5 = T.add_node("A B B")
i6 = T.add_node("B C")
i7 = T.add_node("A A B")
i8 = T.add_node("A B")

# Add edges to the tree
# In connect(), the first argument will become parent and the second argument will become child.

T.connect(i1, i2)
T.connect(i1, i3)
T.connect(i1, i4)
T.connect(i2, i5)
T.connect(i2, i6)
T.connect(i3, i7)
T.connect(i5, i8)

# Create MS connectivity restraint

ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
ss= IMP.core.DistancePairScore(ub)
r= IMP.core.MSConnectivityRestraint(ss, T)

# Add restraint to the model and evaluate the model score

m.add_restraint(r)
r.add_particles(ps)
m.evaluate(False)
