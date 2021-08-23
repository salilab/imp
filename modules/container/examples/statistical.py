## \example container/statistical.py
# This example shows how to create a simple statistical potential using
# the PredicatePairsRestraint.

import IMP
import IMP.core
import IMP.atom
import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "statistical potential")

# Create simple test model containing 6 spherical particles
m = IMP.Model()
ps = IMP.core.create_xyzr_particles(m, 6, 1.0, 5.0)

# Assign a numeric type to some of the particles. In practice this might
# be a function of the atom and/or residue type.
ik = IMP.IntKey("statpot_type")
ps[0].add_attribute(ik, 0)
ps[1].add_attribute(ik, 1)
ps[2].add_attribute(ik, 0)
ps[3].add_attribute(ik, 1)


# Define a custom PairPredicate that, given a pair of particles, maps the
# pair of types to a unique index, assuming that the i-j interaction is the
# same as j-i
class PairTypePredicate(IMP.PairPredicate):
    def do_get_inputs(self, m, pis):
        # We only use the particles themselves, no other inputs
        return [m.get_particle(i) for i in pis]

    def get_value_index(self, m, pis):
        # Return -1 if either particle is untyped
        if not all(m.get_has_attribute(ik, pi) for pi in pis):
            print("particle pair %s is untyped" % str(pis))
            return -1
        # Particle types 0,0 map to unique index 0; 1,0 or 0,1 to 1; 1,1 to 2
        ts = sorted(m.get_attribute(ik, pi) for pi in pis)
        ind = (ts[1] * ts[1]+1)/2 + ts[0]
        print("particle pair %s types %s map to %d" % (pis, ts, ind))
        return ind


# Apply our restraint to all pairs of particles within 20.0
lps = IMP.container.ListSingletonContainer(m, ps)
nbl = IMP.container.ClosePairContainer(lps, 20.0, 0.2)
r = IMP.container.PredicatePairsRestraint(PairTypePredicate(), nbl)

# Now we can score specific interactions. Here we try to draw pairs
# of particles that both have type 1 (unique index 2 returned by
# our predicate) together
ps = IMP.core.HarmonicDistancePairScore(0., 1.)
r.set_score(2, ps)

# Any other interaction between either typed or untyped particles is ignored
r.set_is_complete(False)

# Score the system
sf = IMP.core.RestraintsScoringFunction([r])
print("Score is", sf.evaluate(False))
