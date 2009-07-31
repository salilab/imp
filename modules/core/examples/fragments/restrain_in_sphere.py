ub= IMP.core.HarmonicUpperBound(radius, stiffness)
ss= IMP.core.DistanceToSingletonScore(ub, center)

r= IMP.core.SingletonsRestraint(ss, pc)
m.add_restraint(r)
