import IMP
import IMP.domino2
import IMP.core

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
    states= IMP.domino2.XYZStates(vector_states)
    #Map states of particles
    for p in ps:
        domino_smp.set_particle_states(p,states)

#### REPRESENTATION
#1. setting up the representation (6 particles)
m=IMP.Model()
m.set_log_level(IMP.SILENT)
ps=IMP.Particles()
for i in range(0,6):
    p=IMP.Particle(m)
    IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0.,0.,0.))
    ps.append(p)

####SCORING
#1. setting up the scoring function
setup_scoring_function(ps)

####OPTIMIZATION
#1. load domino sampler and set required properties
domino_smp=IMP.domino2.DominoSampler(m)
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
