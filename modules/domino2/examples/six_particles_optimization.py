import IMP
import IMP.domino2
import IMP.core
NUM_PARTICLES=6
NUM_STATES=40

#set restraints
def setup_restraints(m, ps):
    pairs=[[0,1],[0,2],[1,2],[2,3],[3,4],[4,5],[3,5]]
    sf = IMP.core.Harmonic(1.0, 0.5)
    for pair in pairs:
        r=IMP.core.DistanceRestraint(sf, ps[pair[0]], ps[pair[1]])
        m.add_restraint(r)

IMP.set_log_level(IMP.TERSE)
m=IMP.Model()
print "create sampler"
s=IMP.domino2.DominoSampler(m)
#1. set up the particles
print "setting up particles"
ps=IMP.Particles()
for i in range(NUM_PARTICLES):
    p=IMP.Particle(m)
    IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0.,0.,0.))
    ps.append(p)

#2. set up the discrete set of states
print "setting up a discrete set of states"
lb=IMP.algebra.Vector3D(-10.,-10.,-10.)
ub=IMP.algebra.Vector3D(10.,10.,10.)
bb=IMP.algebra.BoundingBox3D(lb,ub)
vs=[]
for i in range(NUM_STATES):
    vs.append(IMP.algebra.get_random_vector_in(bb))

print "create states"
states= IMP.domino2.XYZsStates(vs)
for p in ps:
    print p.get_name()
    s.set_particle_states(p, states)

#3. add restraints (defining the scoring function)
print "setting up restraints"
setup_restraints(m, ps)

#5. optimize
s.set_maximum_score(1)
print "sampling"
cs=s.get_sample()

print "Found ", cs.get_number_of_configurations(), "solutions"
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    print "solution number:",i," is:", m.evaluate(False)
