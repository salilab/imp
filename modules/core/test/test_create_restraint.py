import IMP.core
import IMP.kernel

m = IMP.kernel.Model()
p0 = IMP.kernel.Particle(m)
p1 = IMP.kernel.Particle(m)

ps = IMP.core.SoftSpherePairScore(1)
cr = IMP.core.create_restraint(ps, (p0, p1))
