## \example core/restrain_minimum_distance.py
## This example shows how to restrain based on the minimum distance between two arbitrary sets of particles. You should also see IMP::atom::create_distance_restraint() for a related helper function.

import IMP
import IMP.core
m= IMP.Model()

# stuff to create some XYZR particles
ds0= IMP.core.create_xyzr_particles(m, 10, 1, 50)
ds1= IMP.core.create_xyzr_particles(m, 10, 1, 50)

# first create a table mapping a sentinenal particle to each set
tref= IMP.core.TableRefiner()
tref.add_particle(ds0[0], ds0)
tref.add_particle(ds1[0], ds1)

# create a pair score to apply to the closest pair
hps=IMP.core.HarmonicSphereDistancePairScore(0, 1)
# create the pair score with this refiner telling it to use the
# single closest particle
ps= IMP.core.KClosePairsPairScore(hps, tref, 1)

# create a restraint by binding the pair score to the sentinal particles
r= IMP.core.PairRestraint(ps, (ds0[0], ds1[0]), "distance")

mc= IMP.core.MonteCarlo(m)
bm= IMP.core.BallMover(ds0+ds1, 1)
mc.add_mover(bm)
mc.set_scoring_function([r])
mc.optimize(1000)

# find out which pair ended up close
for p0 in ds0:
    for p1 in ds1:
        if IMP.core.get_distance(p0, p1) <.1:
            print p0, p1
