## \example example/range_restriction.py
## This example shows how to use the example singleton modifier to constrain the
## coordinates of a set of particles to remain within a box.

import IMP.example
import IMP.core
import IMP.container

bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))

m= IMP.Model()
ps= IMP.core.create_xyzr_particles(m, 20, 1)
sc= IMP.container.ListSingletonContainer(ps)

# apply the range restriction modifier to each each particle in sc
ss= IMP.container.SingletonsConstraint(IMP.example.ExampleSingletonModifier(bb), None, sc)
m.add_score_state(ss)

# now optimize and things
# ...
