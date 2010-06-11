import IMP
import IMP.domino2
import IMP.core

#set restraints
def setup_restraints(m, ps):
    pairs=[[0,1],[0,2],[1,2],[2,3],[3,4],[4,5],[3,5]]
    sf = IMP.core.Harmonic(1.0, 1)
    for pair in pairs:
        r=IMP.core.DistanceRestraint(sf, ps[pair[0]], ps[pair[1]])
        m.add_restraint(r)

IMP.set_log_level(IMP.TERSE)
m=IMP.Model()
m.set_log_level(IMP.SILENT)
print "create sampler"
s=IMP.domino2.DominoSampler(m)
#1. set up the particles
print "setting up particles"
ps=IMP.Particles()
for i in range(0,6):
    p=IMP.Particle(m)
    IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0.,0.,0.))
    ps.append(p)

#2. set up the discrete set of states
print "setting up a discrete set of states"
vs=[]
for i in range(0,4):
    vs.append(IMP.algebra.Vector3D(i,0,0))
    vs.append(IMP.algebra.Vector3D(i,1,0))

print "create states"
states= IMP.domino2.XYZsStates(vs)
for p in ps:
    print p.get_name()
    s.set_particle_states(p, states)

#3. add restraints (defining the scoring function)
print "setting up restraints"
setup_restraints(m, ps)

#5. optimize
s.set_maximum_score(.2)
s.set_log_level(IMP.VERBOSE)
print "sampling"
cs=s.get_sample()

print "Found ", cs.get_number_of_configurations(), "solutions"
for i in range(cs.get_number_of_configurations()):
    cs.load_configuration(i)
    print "solution number:",i," is:", m.evaluate(False)
