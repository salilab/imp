import IMP.core
import IMP.container

m = IMP.Model()
p0 = m.add_particle("P0")
p1 = m.add_particle("P1")

ps = IMP.core.SoftSpherePairScore(1)
cr = IMP.container.create_restraint(
    ps, IMP.container.ListPairContainer(m, [(p0, p1)]))
