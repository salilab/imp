import IMP.examples
import IMP.core
import IMP.container

bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                              IMP.algebra.Vector3D(10,10,10))

m= IMP.Model()
ps= IMP.core.create_xyzr_particles(m, 20, 1)
sc= IMP.container.ListSingletonContainer(ps)
ss= IMP.container.SingletonsConstraint(sc, IMP.examples.ExampleSingletonModifier(bb), None)
m.add_score_state(ss)

# now optimize and things
