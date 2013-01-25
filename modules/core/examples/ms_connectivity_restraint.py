## \example core/ms_connectivity_restraint.py
## This example shows how to use the MSConnectivityRestraint to ensure that all the particles that are part of complexes end up in a connected conformation following the optimization. It allows multiple copies of particles and takes an experimental tree as an input.
##

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

# Create MS connectivity restraint

ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
ss= IMP.core.DistancePairScore(ub)
r= IMP.core.MSConnectivityRestraint(ss)

# Add particle types to the restraint
# add_type() returns a unique type handle that can be used as an argument to add_composite() later on.

pa = r.add_type([ds[0], ds[1]])
pb = r.add_type([ds[2], ds[3]])
pc = r.add_type([ds[4]])
pd = r.add_type([ds[5]])

# Enter experimental tree data into restraint
# In add_composite(), the first argument is node label and the second argument is the parent.

i1 = r.add_composite([pa, pa, pb, pb, pc])
i2 = r.add_composite([pa, pb, pb, pc], i1)
i3 = r.add_composite([pa, pa, pb, pb], i1)
i4 = r.add_composite([pa, pb], i1)
i5 = r.add_composite([pa, pb, pb], i2)
i6 = r.add_composite([pb, pc], i2)
i7 = r.add_composite([pa, pa, pb], i3)
i8 = r.add_composite([pa, pb], i5)

# Add restraint to the model and evaluate the model score

m.add_restraint(r)
m.evaluate(False)
