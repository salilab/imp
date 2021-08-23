## \example core/optimize_balls.py
# This example optimizes a set of a balls to form 100 chains packed into a
# box. It illustrates using Monte Carlo (incremental) and conjugate
# gradients in conjunction in a non-trivial optimization.

import IMP.core
import IMP.display
import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "Optimize balls example")

bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                               IMP.algebra.Vector3D(10, 10, 10))
if IMP.get_is_quick_test():
    ni = 2
    nj = 2
    np = 2
    radius = .45
    k = 100
    ncg = 10
    nmc = 1
    ninner = 1
    nouter = 1
else:
    ni = 10
    nj = 10
    np = 10
    radius = .45
    k = 100
    ncg = 1000
    nmc = ni * nj * np * 100
    ninner = 5
    nouter = 11

print(IMP.get_is_quick_test(), ni, nj, np, ninner, nouter)
# using a HarmonicDistancePairScore for fixed length links is more
# efficient than using a HarmonicSphereDistnacePairScore and works
# better with the optimizer
lps = IMP.core.HarmonicDistancePairScore(1.5 * radius, k)
sps = IMP.core.SoftSpherePairScore(k)

m = IMP.Model()
# IMP.set_log_level(IMP.SILENT)
aps = []
filters = []
movers = []
restraints = []
for i in range(0, ni):
    for j in range(0, nj):
        base = IMP.algebra.Vector3D(i, j, 0)
        chain = []
        for k in range(0, np):
            p = IMP.Particle(m)
            p.set_name("P" + str(i) + " " + str(j) + " " + str(k))
            s = IMP.algebra.Sphere3D(
                IMP.algebra.get_random_vector_in(bb), radius)
            d = IMP.core.XYZR.setup_particle(p, s)
            d.set_coordinates_are_optimized(True)
            movers.append(IMP.core.BallMover(m, p, radius * 2))
            movers[-1].set_was_used(True)
            IMP.display.Colored.setup_particle(
                p, IMP.display.get_display_color(i * nj + j))
            if k == 0:
                d.set_coordinates(base)
            else:
                d.set_coordinates_are_optimized(True)
            chain.append(p)
            aps.append(p)
        # set up a chain of bonds
        cpc = IMP.container.ExclusiveConsecutivePairContainer(m, chain)
        r = IMP.container.PairsRestraint(lps, cpc)
        restraints.append(r)

# don't apply excluded volume to consecutive particles
filters.append(IMP.container.ExclusiveConsecutivePairFilter())
ibss = IMP.core.BoundingBox3DSingletonScore(
    IMP.core.HarmonicUpperBound(0, k), bb)
bbr = IMP.container.SingletonsRestraint(ibss, aps)
restraints.append(bbr)

cg = IMP.core.ConjugateGradients(m)
mc = IMP.core.MonteCarlo(m)
mc.set_name("MC")
sm = IMP.core.SerialMover(movers)
mc.add_mover(sm)
# we are special casing the nbl term
isf = IMP.core.IncrementalScoringFunction(m, aps, restraints)
isf.set_name("I")
# use special incremental support for the non-bonded part
# apply the pair score sps to all touching ball pairs from the list
# of particles aps, using the filters to remove undersired pairs
# this is equivalent to the nbl construction above but optimized for
# incremental
isf.add_close_pair_score(sps, 0, aps, filters)

# create a scoring function for conjugate gradients that includes the
# ExcludedVolumeRestraint
nbl = IMP.core.ExcludedVolumeRestraint(aps, k, 1)
nbl.set_pair_filters(filters)
sf = IMP.core.RestraintsScoringFunction(restraints + [nbl], "RSF")

if True:
    mc.set_incremental_scoring_function(isf)
else:
    # we could, instead do non-incremental scoring
    mc.set_scoring_function(sf)

# first relax the bonds a bit
rs = []
for p in aps:
    rs.append(IMP.ScopedSetFloatAttribute(p, IMP.core.XYZR.get_radius_key(),
                                          0))
cg.set_scoring_function(sf)
cg.optimize(ncg)
for r in restraints:
    print(r.get_name(), r.evaluate(False))

# shrink each of the particles, relax the configuration, repeat
for i in range(1, nouter):
    rs = []
    factor = .1 * i
    for p in aps:
        rs.append(
            IMP.ScopedSetFloatAttribute(
                p, IMP.core.XYZR.get_radius_key(),
                IMP.core.XYZR(p).get_radius() * factor))
    # move each particle nmc times
    print(factor)
    for j in range(0, ninner):
        print("stage", j)
        mc.set_kt(100.0 / (3 * j + 1))
        print("mc", mc.optimize((j + 1) * nmc), cg.optimize(nmc))
    del rs
    for r in restraints:
        print(r.get_name(), r.evaluate(False))

w = IMP.display.PymolWriter("final.pym")
for p in aps:
    g = IMP.core.XYZRGeometry(p)
    w.add_geometry(g)
g = IMP.display.BoundingBoxGeometry(bb)
g.set_name("bb")
w.add_geometry(g)
