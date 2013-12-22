import IMP.core
import IMP.container
import IMP.kernel

m = IMP.kernel.Model()
p0 = IMP.kernel.Particle(m)
p1 = IMP.kernel.Particle(m)

ps = IMP.core.SoftSpherePairScore(1)
cr = IMP.container.create_restraint(
    ps, IMP.container.ListPairContainer([(p0, p1)]))
