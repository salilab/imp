import IMP
import IMP.core
import IMP.misc
m= IMP.Model()

# Put the parent particles for each molecule
hs=IMP.Particles()

# create the molecules
for i in range(0,10):
    p=IMP.Particle()
    m.add_particle(p)
    d= IMP.core.MolecularHierarchyDecorator.create(p)
    d.set_type(IMP.core.MolecularHierarchyDecorator.PROTEIN)
    for j in range(0,5):
        p=IMP.Particle()
        m.add_particle(p)
        cd= IMP.core.MolecularHierarchyDecorator.create(p)
        cd.set_type(IMP.core.MolecularHierarchyDecorator.FRAGMENT)
        d.add_child(cd)
        xd= IMP.core.XYZRDecorator.create(p)
        xd.set_coordinates(IMP.Vector3D(3*i,j,0))
        xd.set_radius(1)
    hs.append(p)

ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0,1))
cps= IMP.core.ChildrenParticleRefiner()
lrps = IMP.misc.LowestRefinedPairScore(cps,ps)
cr = IMP.core.ConnectivityRestraint(lrps)
cr.set_particles(hs)
m.add_restraint(cr)

m.evaluate(False)
