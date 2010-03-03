import IMP.example
radius=10
stiffness=2
center= IMP.algebra.Vector3D(1,2,3)
(m,c)=IMP.example.create_model_and_particles()
ub= IMP.core.HarmonicUpperBound(radius, stiffness)
ss= IMP.core.DistanceToSingletonScore(ub, center)

r= IMP.container.SingletonsRestraint(ss, c)
m.add_restraint(r)
m.evaluate(False)
