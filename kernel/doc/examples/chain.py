import IMP
import random

#IMP.set_log_level(IMP.VERBOSE)
radius =1.0
rk= IMP.FloatKey("radius")
m= IMP.Model()
pts= IMP.Particles()
for i in range(0,20):
    p= IMP.Particle()
    pi= m.add_particle(p)
    d= IMP.XYZDecorator.create(p)
    d.set_x(random.uniform(0,10))
    d.set_y(random.uniform(0,10))
    d.set_z(random.uniform(0,10))
    d.set_coordinates_are_optimized(True)
    p.add_attribute(rk, radius, False)
    if (i != 0):
        bp= IMP.BondedDecorator.create(p)
        bpr= IMP.BondedDecorator.cast(pts.back())
        b= IMP.custom_bond(bp, bpr, 1.5*radius, 10)
    pts.append(p)
# if you want to inspect the particles
# notice that each bond is a particle
for p in m.get_particles():
    p.show()
# set up the nonbonded list
nbl= IMP.AllSphereNonbondedListScoreState(pts, rk)
nbli= m.add_score_state(nbl)
bl= IMP.BondDecoratorListScoreState(pts)
bli= nbl.add_bonded_list(bl)
# set up excluded volume
ps= IMP.SphereDistancePairScore(IMP.HarmonicLowerBound(0,1),
                                rk)
evr= IMP.NonbondedRestraint(nbl, ps, 1)
evri= m.add_restraint(evr)
# restraint for bonds
br= IMP.BondDecoratorRestraint(bl, IMP.Harmonic(0,1))
bri= m.add_restraint(br)

# just for fun to make the chain straight
ats= IMP.AngleTripletScore(IMP.Harmonic(3.1415, .1))
ar= IMP.TripletChainRestraint(ats)
ar.add_chain(pts)
ari= m.add_restraint(ar)

p= IMP.ParticlePair(pts[0], pts[-1])
pps= IMP.ParticlePairs()
pps.append(p)
cr= IMP.PairListRestraint(pps,
                          IMP.SphereDistancePairScore(IMP.Harmonic(3,1),
                                                      rk))
cri=m.add_restraint(cr)

# set up optimizer
o= IMP.ConjugateGradients()
o.set_model(m)

# not elegant, but it works
vrml= IMP.VRMLLogOptimizerState("state.%03d.vrml", pts)
vrml.set_radius(rk)
vrml.update()
vrml.set_skip_steps(100)
o.add_optimizer_state(vrml)

o.optimize(1000)
# write the final state out
vrml.write_next_file()
