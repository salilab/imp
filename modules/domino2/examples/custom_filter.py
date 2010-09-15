import IMP
import IMP.domino2
import IMP.core

#set restraints
def create_scoring(m, ps):
    pairs=[[0,1],[0,2],[1,3],[2,3],[3,4],[4,5],[1,5]]
    score= IMP.core.HarmonicDistancePairScore(1, 1)
    # the restraint will be broken apart during optimization
    pc= IMP.container.ListPairContainer([(ps[p[0]], ps[p[1]]) for p in pairs],
                                         "Restrained pairs")
    pr= IMP.container.PairsRestraint(score, pc)
    m.set_maximum_score(pr, .01)
    m.add_restraint(pr)
    d= IMP.core.DistanceToSingletonScore(IMP.core.HarmonicUpperBound(2,1),
                                         IMP.algebra.Vector3D(2,0,0))
    return m.get_restraints()

def create_representation(m):
    ps=[]
    for i in range(0,6):
        p=IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0.,0.,0.))
        ps.append(p)
    return ps

def create_discrete_states(ps):
    pst= IMP.domino2.ParticleStatesTable()
    vs=[IMP.algebra.Vector3D(1,0,0),
        IMP.algebra.Vector3D(0,1,0),
        IMP.algebra.Vector3D(1,1,0),
        IMP.algebra.Vector3D(2,1,0),
        IMP.algebra.Vector3D(2,0,0)]
    vs= vs+[-v for v in vs]
    print len(vs), "states for each particle"
    print vs[1]
    states= IMP.domino2.XYZStates(vs)
    # special case ps[0] to remove a sliding degree of freedom
    for p in ps[1:]:
        print p.get_name()
        pst.set_particle_states(p, states)
    return pst

# force particle p to be in state s
class MyFilterTable(IMP.domino2.SubsetFilterTable):
    class MyFilter(IMP.domino2.SubsetFilter):
        def __init__(self, pos, value):
            #print "Filtering with", pos, value
            IMP.domino2.SubsetFilter.__init__(self, "MF"+str(pos) + " " +str(value))
            self.pos=pos
            self.value=value
        def get_is_ok(self, state):
            ret= state[self.pos]==self.value
            return ret
        def get_strength(self):
            # return the maximum value since it dictates the position
            return 1
    def __init__(self, p, s):
        IMP.domino2.SubsetFilterTable.__init__(self, "MFT"+p.get_name()+" at "+str(s))
        self.p=p
        self.s=s
    def get_subset_filter(self, subset, excluded):
        # create a filter if self.p is in subset but not in excluded
        if self.p in subset and self.p not in sum([list(x) for x in excluded], []):
            # pass the position of self.p and the value that it must have
            return self.MyFilter(list(subset).index(self.p), self.s)
        else:
            return None

def create_sampler(m, ps, pst):
    s=IMP.domino2.DominoSampler(m, pst)
    s.set_log_level(IMP.VERBOSE)
    # the following lines recreate the defaults and so are optional
    me= IMP.domino2.ModelSubsetEvaluatorTable(m, pst)
    s.set_subset_evaluator_table(me)
    filters=[]
    # do not allow particles with the same ParticleStates object
    # to have the same state index
    filters.append(IMP.domino2.PermutationSubsetFilterTable(pst))
    # filter states that score worse than the cutoffs in the Model
    filters.append(IMP.domino2.RestraintScoreSubsetFilterTable(me))
    filters[-1].set_log_level(IMP.SILENT)
    mf=MyFilterTable(ps[1], 0)
    # try with and without this line
    filters.append(mf)
    states= IMP.domino2.BranchAndBoundSubsetStatesTable(pst, filters)
    #states.set_log_level(IMP.SILENT);
    s.set_subset_states_table(states)
    return s

IMP.set_log_level(IMP.TERSE)
m=IMP.Model()
m.set_log_level(IMP.SILENT)

print "creating representation"
ps=create_representation(m)
print "creating discrete states"
pst=create_discrete_states(ps)
print "creating sampler"
s=create_sampler(m, ps, pst)
print "creating score function"
rs=create_scoring(m, ps)

print "sampling"
cs=s.get_sample()

print "found ", cs.get_number_of_configurations(), "solutions"
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    print "solution number:",i," is:", m.evaluate(False)
    for p in ps:
        print IMP.core.XYZ(p).get_x(), IMP.core.XYZ(p).get_y()
