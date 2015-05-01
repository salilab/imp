import IMP.core

IMP.set_deprecation_exceptions(True)

m = IMP.Model()
p0 = IMP.Particle(m)
p1 = IMP.Particle(m)

ps = IMP.core.SoftSpherePairScore(1)
cr = IMP.core.create_restraint(ps, (p0, p1))
