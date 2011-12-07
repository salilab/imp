import IMP.core
import IMP.display
import IMP.container
import IMP.rmf

bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                             IMP.algebra.Vector3D(10,10,10));
ni=10
nj=10
np=10
radius=.45
k=100

# using a HarmonicDistancePairScore for fixed length links is more
# efficient than using a HarmonicSphereDistnacePairScore and works
# better with the optimizer
lps= IMP.core.HarmonicDistancePairScore(1.5*radius, k)
sps= IMP.core.SoftSpherePairScore(k)

m= IMP.Model()

aps=[]
filters=[]
rs=[]
movers=[]
rss= IMP.RestraintSet("bonds")
m.add_restraint(rss)
for i in range(0,ni):
    for j in range(0,nj):
        base=IMP.algebra.Vector3D(i,j,0)
        chain=[]
        for k in range(0,np):
            p= IMP.Particle(m)
            p.set_name("P"+str(i)+" "+str(j)+" "+str(k))
            s=IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), radius)
            d= IMP.core.XYZR.setup_particle(p,s)
            movers.append(IMP.core.BallMover([p], radius*2))
            movers[-1].set_was_used(True)
            IMP.display.Colored.setup_particle(p, IMP.display.get_display_color(i*nj+j))
            if k==0:
                d.set_coordinates(base)
            else:
                d.set_coordinates_are_optimized(True)
            chain.append(p)
            aps.append(p)
        cpc= IMP.container.ConsecutivePairContainer(chain, True)
        r= IMP.container.PairsRestraint(lps, cpc)
        rs.append(r)
        rss.add_restraint(r)
# cheat
filters.append(IMP.container.InContainerPairFilter(cpc))
filters[-1].set_was_used(True)
laps=IMP.container.ListSingletonContainer(aps)
nbl= IMP.core.ExcludedVolumeRestraint(laps,
                                      k, 1)
nbl.set_pair_filters(filters)
m.add_restraint(nbl)
ibss= IMP.core.BoundingBox3DSingletonScore(IMP.core.HarmonicUpperBound(0,k), bb)
bbr= IMP.container.SingletonsRestraint(ibss, laps)
rs.append(bbr)
m.add_restraint(bbr)

cg= IMP.core.ConjugateGradients(m)
mc=IMP.core.MonteCarlo(m)
sm= IMP.core.SerialMover(movers)
mc.add_mover(sm)
# we are special casing the nbl term
mc.set_restraints(rs)
mc.set_use_incremental_evaluate(True)
# use special incremental support for the non-bonded part
mc.set_close_pair_score(sps, 0, aps, filters)

m.set_log_level(IMP.VERBOSE)

# first relax the bonds a bit
rs=[]
for p in aps:
    rs.append(IMP.ScopedSetFloatAttribute(p, IMP.core.XYZR.get_radius_key(),
                                          0))
cg.optimize(1000)
print "collisions", nbl.evaluate(False), "bonds", rss.evaluate(False), bbr.evaluate(False)

# shrink each of the particles, relax the configuration, repeat
for i in range(1,11):
    rs=[]
    factor=.1*i
    for p in aps:
        rs.append(IMP.ScopedSetFloatAttribute(p, IMP.core.XYZR.get_radius_key(),
                                         IMP.core.XYZR(p).get_radius()*factor))
    # move each particle 100 times
    print factor
    for j in range(0,5):
        mc.set_kt(100.0/(3*j+1))
        print "mc", mc.optimize(ni*nj*np*(j+1)*100), m.evaluate(False), cg.optimize(10)
    del rs
    print "collisions", nbl.evaluate(False), "bonds", rss.evaluate(False), "bounding box", bbr.evaluate(False)

w= IMP.display.PymolWriter("final.pym")
for p in aps:
    g= IMP.core.XYZRGeometry(p)
    w.add_geometry(g)
g= IMP.display.BoundingBoxGeometry(bb)
g.set_name("bb")
w.add_geometry(g)
