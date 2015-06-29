import IMP.core
import IMP.container

m = IMP.Model()
p0 = IMP.Particle(m)
p1 = IMP.Particle(m)

ps = IMP.core.SoftSpherePairScore(1)
cr = IMP.container.create_restraint(
    ps, IMP.container.ListPairContainer([(p0, p1)]))
