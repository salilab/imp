## \example domino/six_particles_optimization.py
# Optimize six particles on a 2D unit grid. In order to remove translation
# degrees of freedom, the 0th particle is pinned at the origin by allowing
# it only a single conformation. To remove flips, the first particle is
# restrained to have a positive x coordinate.

from __future__ import print_function
import IMP
import IMP.domino
import IMP.core
import IMP.container
import sys

IMP.setup_from_argv(sys.argv, "six particles optimization")

# set restraints


def create_scoring(m, ps):
    pairs = [[0, 1], [0, 2], [1, 3], [2, 3], [3, 4], [4, 5], [1, 5]]
    # we will restrain various pairs to be 1 apart
    score = IMP.core.HarmonicDistancePairScore(1, 1)
    # the restraint will be broken apart during optimization
    # map the indices above to actual particles
    pc = IMP.container.ListPairContainer(m,
                                         [(ps[p[0]], ps[p[1]]) for p in pairs],
                                         "Restrained pairs")
    pr = IMP.container.PairsRestraint(score, pc)
    pr.set_maximum_score(.01)
    d = IMP.core.DistanceToSingletonScore(IMP.core.HarmonicUpperBound(2, 1),
                                          IMP.algebra.Vector3D(2, 0, 0))
    # force ps[1] to be on the positive side to remove flip degree of freedom
    dr = IMP.core.SingletonRestraint(m, d, ps[1])
    # we are not interested in conformations which don't fit the distances
    # exactly, but using 0 is tricky
    dr.set_maximum_score(.01)
    return [pr, dr]


def create_representation(m):
    ps = []
    # create size particles, initial coordinates don't matter.
    for i in range(0, 6):
        p = m.add_particle("P%d" % i)
        IMP.core.XYZ.setup_particle(m, p, IMP.algebra.Vector3D(i, 0., 0.))
        ps.append(p)
    return ps


def create_discrete_states(m, ps):
    pst = IMP.domino.ParticleStatesTable()
    vs = [IMP.algebra.Vector3D(1, 0, 0),
          IMP.algebra.Vector3D(0, 1, 0),
          IMP.algebra.Vector3D(1, 1, 0),
          IMP.algebra.Vector3D(2, 1, 0),
          IMP.algebra.Vector3D(2, 0, 0)]
    vs = vs + [-v for v in vs]
    print(len(vs), "states for each particle")
    states = IMP.domino.XYZStates(vs)
    # special case ps[0] to remove a sliding degree of freedom
    # all other particles are given the same set of states
    for p in ps[1:]:
        pst.set_particle_states(m.get_particle(p), states)
    return pst


def create_sampler(m, r, pst):
    # create the sampler and pass it the states for each patricle
    s = IMP.domino.DominoSampler(m, pst)
    s.set_restraints(r)
    # the following lines recreate the defaults and so are optional
    filters = []
    # create a restraint cache to avoid re-evaluating restraints
    rc = IMP.domino.RestraintCache(pst)
    # add the list of restraints we want to use
    rc.add_restraints(r)
    # do not allow particles with the same ParticleStates object
    # to have the same state index
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    # filter states that score worse than the cutoffs in the Model
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(rc))
    filters[-1].set_log_level(IMP.SILENT)
    # try to be intelligent about enumerating the states in each subset
    states = IMP.domino.BranchAndBoundAssignmentsTable(pst, filters)
    states.set_log_level(IMP.SILENT)
    s.set_assignments_table(states)
    s.set_subset_filter_tables(filters)

    return s


IMP.set_log_level(IMP.TERSE)
m = IMP.Model()
# don't print information during Model.evaluate
m.set_log_level(IMP.SILENT)

print("creating representation")
ps = create_representation(m)
print("creating discrete states")
pst = create_discrete_states(m, ps)
print("creating score function")
rs = create_scoring(m, ps)
print("creating sampler")
s = create_sampler(m, rs, pst)

print("sampling")
# get an IMP.ConfigurationSet with the sampled states. If there are very
# many, it might be better to use s.get_sample_states() and then
# IMP.domino.load_particle_states() to handle the states as that takes
# much less memory, and time.
cs = s.create_sample()

print("found ", cs.get_number_of_configurations(), "solutions")
sf = IMP.core.RestraintsScoringFunction(rs)
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    print("solution number:", i, " is:", sf.evaluate(False))
    for p in ps:
        print(IMP.core.XYZ(m, p).get_x(), IMP.core.XYZ(m, p).get_y())
