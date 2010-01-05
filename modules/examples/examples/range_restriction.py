import IMP.examples
import IMP.core

bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))

m= IMP.Model()
ps= IMP.core.create_xyzr_particles(m, 20, 1)
sc= IMP.core.ListSingletonContainer(ps)
ss= IMP.core.SingletonsConstraint(sc, IMP.examples.ExampleSingletonModifier(bb), None)
m.add_score_state(ss)

# now optimize and things
