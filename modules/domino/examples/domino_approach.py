## \example domino/domino_approach.py
## Optimize six particles

#### NOT FULLY IMPLEMENTED YET!
import IMP
import IMP.domino
import IMP.core
import sys

def optimize_subsets(subsets):
    pst= IMP.domino.ParticleStatesTable()
    for subset in subsets:
        None
        #TODO - do the actual sampling
    #combine back to ParticleStatesTable
    #there is now a write_particles_binary (multiple states into one file)
    return pst

def get_subsets(ps):
    mdl=ps[0].get_model()
    inter_graph=IMP.domino.get_interaction_graph(mdl.get_root_restraint_set(), ps)
    jt=IMP.domino.get_junction_tree(inter_graph)
    return IMP.domino.get_subsets(jt)


def setup_scoring_function(ps):
    m=ps[0].get_model()
    pairs=[[0,1],[0,2],[1,2],[2,3],[3,4],[4,5],[3,5]]
    sf = IMP.core.Harmonic(1.0, 1)
    for pair in pairs:
        r=IMP.core.DistanceRestraint(sf, ps[pair[0]], ps[pair[1]])
        m.add_restraint(r)

#Initiate a set of states for each particle in ps
def initiate_configuration(domino_smp,ps):
    #Generate a discrete set of states
    vector_states=[]
    for i in range(0,4):
        vector_states.append(IMP.algebra.Vector3D(i,0,0))
        vector_states.append(IMP.algebra.Vector3D(i,1,0))
    #Generate a discrete set of states for each of the particles
    states= IMP.domino.XYZStates(vector_states)
    #Map states of particles
    for p in ps:
        domino_smp.set_particle_states(p,states)

sys.exit()
#### REPRESENTATION
#1. setting up the representation (6 particles)
mdl=IMP.Model()
mdl.set_log_level(IMP.base.SILENT)
ps=[]
for i in range(0,6):
    p=IMP.Particle(mdl)
    IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0.,0.,0.))
    ps.append(p)

####SCORING
#1. setting up the scoring function
setup_scoring_function(ps)

#1. get the subsets
subsets=get_subsets(ps)

#optimize each one (returning ParticleStatesTable)
pst = optimize_subsets(subsets)
sys.exit()

#subsets=[]

#jt.show()

#2. sample each subset
#3. gathering


####OPTIMIZATION
#1. load domino sampler and set required properties
domino_smp=IMP.domino.DominoSampler(m)
domino_smp.set_maximum_score(.2)

#2. initiate configuration
initiate_configuration(domino_smp,ps)


#3. construct subset of variables AND
#   optimize subsets of variables subject to the discrete sampling space AND
#   gather subset of variables into global solutions
cs=domino_smp.get_sample()

####ANALYSE
#4. report solutions
print "Found ", cs.get_number_of_configurations(), "solutions"
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    #print the configuration:
    print "solution number:",i," scored:", m.evaluate(False)
