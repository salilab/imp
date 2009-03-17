uf= IMP.core.Harmonic(0,1)
ps= IMP.core.SphereDistancePairScore(uf)
r= IMP.core.PairRestraint(ps, ps.get_particle(0), ps.get_particle(1))
m.add_restraint(r)
