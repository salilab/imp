## \example domino/custom_filter.py
## This example looks like the six particle optimization example except a filter is used instead of a restraint to remove the flip degree of freedom. The filter is written is python, which makes for quick prototyping, but slow run times.

import IMP
import IMP.domino
import IMP.core
import IMP.container

#set restraints
def create_scoring(m, ps):
    pairs=[[0,1],[0,2],[1,3],[2,3],[3,4],[4,5],[1,5]]
    score= IMP.core.HarmonicDistancePairScore(1, 1)
    # the restraint will be broken apart during optimization
    pc= IMP.container.ListPairContainer([(ps[p[0]], ps[p[1]]) for p in pairs],
                                         "Restrained pairs")
    pr= IMP.container.PairsRestraint(score, pc)
    pr.set_maximum_score(.01)
    pr.set_model(m)
    d= IMP.core.DistanceToSingletonScore(IMP.core.HarmonicUpperBound(2,1),
                                         IMP.algebra.Vector3D(2,0,0))
    return [pr]

def create_representation(m):
    ps=[]
    for i in range(0,6):
        p=IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(i,0.,0.))
        ps.append(p)
    return ps

def create_discrete_states(ps):
    pst= IMP.domino.ParticleStatesTable()
    vs=[IMP.algebra.Vector3D(1,0,0),
        IMP.algebra.Vector3D(0,1,0),
        IMP.algebra.Vector3D(1,1,0),
        IMP.algebra.Vector3D(2,1,0),
        IMP.algebra.Vector3D(2,0,0)]
    vs= vs+[-v for v in vs]
    print len(vs), "states for each particle"
    print vs[1]
    states= IMP.domino.XYZStates(vs)
    # special case ps[0] to remove a sliding degree of freedom
    for p in ps[1:]:
        pst.set_particle_states(p, states)
    return pst

# force particle p to be in state s
class MyFilterTable(IMP.domino.SubsetFilterTable):
    class MyFilter(IMP.domino.SubsetFilter):
        def __init__(self, pos, value):
            #print "Filtering with", pos, value
            IMP.domino.SubsetFilter.__init__(self, "MF"+str(pos) + " " +str(value))
            self.pos=pos
            self.value=value
        def get_next_state(self, pos, s):
            # suggest that the sampler try the correct state
            # this method is only called if the filter failed, so pos must be
            # self.pos
            if s[self.pos] > self.value:
                # a very large number
                return 2**29
            else:
                return self.value
        def get_is_ok(self, state):
            # it is only OK if it has the required state
            ret= state[self.pos]==self.value
            return ret
    def get_strength(self, s, excluded):
        # return the maximum value since it dictates the state for the particle
        return 1
    def __init__(self, p, s):
        # set the name of the object to something vaguely useful
        IMP.domino.SubsetFilterTable.__init__(self, "MFT"+p.get_name()+" at "+str(s))
        self.p=p
        self.s=s
    def get_subset_filter(self, subset, excluded):
        # create a filter if self.p is in subset but not in excluded
        if self.p in subset and self.p not in sum([list(x) for x in excluded], []):
            # pass the position of self.p and the value that it must have
            return self.MyFilter(list(subset).index(self.p), self.s)
        else:
            return None


def create_sampler(m, ps, rs, pst):
    s=IMP.domino.DominoSampler(m, pst)
    #s.set_log_level(IMP.base.VERBOSE)
    # the following lines recreate the defaults and so are optional
    filters=[]
    # do not allow particles with the same ParticleStates object
    # to have the same state index
    filters.append(IMP.domino.ExclusionSubsetFilterTable(pst))
    rc= IMP.domino.RestraintCache(pst)
    rc.add_restraints(rs)
    # filter states that score worse than the cutoffs in the Model
    filters.append(IMP.domino.RestraintScoreSubsetFilterTable(rc))
    filters[-1].set_log_level(IMP.base.SILENT)
    mf=MyFilterTable(ps[1], 0)
    # try with and without this line
    filters.append(mf)
    states= IMP.domino.BranchAndBoundAssignmentsTable(pst, filters)
    #states.set_log_level(IMP.base.SILENT);
    s.set_assignments_table(states)
    s.set_subset_filter_tables(filters)
    return s

IMP.base.set_log_level(IMP.base.TERSE)
m=IMP.Model()
m.set_log_level(IMP.base.SILENT)

print "creating representation"
ps=create_representation(m)
print "creating discrete states"
pst=create_discrete_states(ps)
print "creating score function"
rs=create_scoring(m, ps)

print "creating sampler"
s=create_sampler(m, ps, rs, pst)

#s.set_log_level(IMP.base.SILENT)
print "sampling"
cs=s.get_sample()

print "found ", cs.get_number_of_configurations(), "solutions"
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    print "solution number:",i," is:", m.evaluate(False)
    for p in ps:
        print IMP.core.XYZ(p).get_x(), IMP.core.XYZ(p).get_y()
