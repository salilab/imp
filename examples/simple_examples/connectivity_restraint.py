import IMP
import IMP.core
import IMP.misc
import IMP.algebra
import IMP.atom

m= IMP.Model()

# Put the parent particles for each molecule
hs=IMP.Particles()

# create the molecules
for i in range(0,10):
    p=IMP.Particle(m)
    d= IMP.atom.MolecularHierarchyDecorator.create(p,
       IMP.atom.MolecularHierarchyDecorator.PROTEIN)
    for j in range(0,5):
        p=IMP.Particle(m)
        cd= IMP.atom.MolecularHierarchyDecorator.create(p,
            IMP.atom.MolecularHierarchyDecorator.FRAGMENT)
        d.add_child(cd)
        xd= IMP.core.XYZRDecorator.create(p, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(3*i,j,0), 1))
    hs.append(p)

ps= IMP.core.SphereDistancePairScore(IMP.core.HarmonicUpperBound(0,1))
cps= IMP.core.ChildrenRefiner(IMP.atom.MolecularHierarchyDecorator.get_traits())
lrps = IMP.misc.LowestRefinedPairScore(cps,ps)
cr = IMP.core.ConnectivityRestraint(lrps)
cr.set_particles(hs)
m.add_restraint(cr)

m.evaluate(False)
