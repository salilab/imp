## \example core/optimize_balls.py
## This example optimizes a set of a balls to form 100 chains packed into a box. It illustrates using Monte Carlo (incremental) and conjugate gradients in conjunction in a non-trivial optimization.

import IMP.core
import IMP.display
import IMP.container
import IMP.rmf

bb=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                             IMP.algebra.Vector3D(10,10,10));
# in fast do 10,10,10, for the purposes of testing we reduce it
ni=2
nj=2
np=2
radius=.45
k=100

# using a HarmonicDistancePairScore for fixed length links is more
# efficient than using a HarmonicSphereDistnacePairScore and works
# better with the optimizer
lps= IMP.core.HarmonicDistancePairScore(1.5*radius, k)
sps= IMP.core.SoftSpherePairScore(k)

m= IMP.Model()
IMP.base.set_log_level(IMP.base.SILENT)
aps=[]
filters=[]
movers=[]
rss= IMP.RestraintSet(m, 1.0, "bonds")
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
        # set up a chain of bonds
        cpc= IMP.container.ExclusiveConsecutivePairContainer(chain)
        r= IMP.container.PairsRestraint(lps, cpc)
        rss.add_restraint(r)

# don't apply excluded volume to consecutive particles
filters.append(IMP.container.ExclusiveConsecutivePairFilter())
ibss= IMP.core.BoundingBox3DSingletonScore(IMP.core.HarmonicUpperBound(0,k), bb)
bbr= IMP.container.SingletonsRestraint(ibss, aps)
rss.add_restraint(bbr)

cg= IMP.core.ConjugateGradients(m)
mc=IMP.core.MonteCarlo(m)
mc.set_name("MC")
sm= IMP.core.SerialMover(movers)
mc.add_mover(sm)
# we are special casing the nbl term
isf= IMP.core.IncrementalScoringFunction(aps, [rss])
isf.set_name("I")
# use special incremental support for the non-bonded part
# apply the pair score sps to all touching ball pairs from the list of particles
# aps, using the filters to remove undersired pairs
# this is equivalent to the nbl construction above but optimized for incremental
isf.add_close_pair_score(sps, 0, aps, filters)

# create a scoring function for conjugate gradients that includes the
# ExcludedVolumeRestraint
nbl= IMP.core.ExcludedVolumeRestraint(aps, k, 1)
nbl.set_pair_filters(filters)
sf= IMP.core.RestraintsScoringFunction([rss, nbl], "RSF")

if True:
    mc.set_incremental_scoring_function(isf)
else:
    # we could, instead do non-incremental scoring
    mc.set_scoring_function(sf)

# first relax the bonds a bit
rs=[]
for p in aps:
    rs.append(IMP.ScopedSetFloatAttribute(p, IMP.core.XYZR.get_radius_key(),
                                          0))
cg.set_scoring_function(sf)
cg.optimize(1000)
print "collisions", nbl.evaluate(False), "bonds", rss.evaluate(False),
print bbr.evaluate(False)

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
        print "stage", j
        isf.set_log_level(IMP.base.TERSE)
        mc.set_kt(100.0/(3*j+1))
        print "mc", mc.optimize(ni*nj*np*(j+1)*100), m.evaluate(False), cg.optimize(10)
    del rs
    print "collisions", nbl.evaluate(False), "bonds", rss.evaluate(False),
    print "bounding box", bbr.evaluate(False)

w= IMP.display.PymolWriter("final.pym")
for p in aps:
    g= IMP.core.XYZRGeometry(p)
    w.add_geometry(g)
g= IMP.display.BoundingBoxGeometry(bb)
g.set_name("bb")
w.add_geometry(g)
