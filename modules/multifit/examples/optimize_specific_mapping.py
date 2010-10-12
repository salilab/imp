# This example is still under development, and does not yet run correctly.

#load the representation file
#load the restraints file
#build a junction tree from a restraint file

import IMP.multifit
import IMP.domino2

#returns the restraints
def setup_scoring_function(ps):
    m=ps[0].get_model()
    pairs=[[0,1],[0,2],[0,3]]
    restraints=IMP.Restraints()
    for pair in pairs:
        rbs=IMP.core.RigidBodies()
        rbs.append(IMP.core.RigidBody(ps[pair[0]]))
        rbs.append(IMP.core.RigidBody(ps[pair[1]]))
        ex_r=IMP.helper.create_simple_excluded_volume_on_rigid_bodies(
            rbs,
            IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits()))
        m.add_restraint(ex_r.get_restraint())
        restraints.append(ex_r)
    return restraints

'''
def display_scoring_function(ps):
    mdl=ps[0].get_model()
    inter_g  = IMP.domino2.get_interaction_graph(ps,[mdl.get_root_restraint_set()])
    inter_g_geo=IMP.domino2.get_interaction_graph_geometry(inter_g)
    w = IMP.display.ChimeraWriter("out.py")
    w.add_geometry(inter_g_geo)
    del w
    #TODO - does not really work....
    # w1 = IMP.display.ChimeraWriter("out2.py")
    # jt=IMP.domino2.get_junction_tree(inter_g)
    # jt_geo=IMP.domino2.get_subset_graph_geometry(jt)
    # w1.add_geometry(jt_geo)
    # del w1
'''

IMP.set_log_level(IMP.TERSE)
settings = IMP.multifit.read_settings(IMP.multifit.get_example_path("input/assembly.input"),
  IMP.multifit.get_example_path("input"))
data = IMP.multifit.DataContainer(settings)
mdl=data.get_model()
ps=IMP.Particles()
for i in range(data.get_number_of_components()):
    ps.append(data.get_component(i))
restraints=setup_scoring_function(ps)

#set the discrete sampling space
#1.1 populate the transformations
transformations=[]
states_of_particle=[]
pst= IMP.domino2.ParticleStatesTable()
for i in range(data.get_number_of_components()):
    p=data.get_component(i)
    rb=IMP.core.RigidBody(p)
    transformations.append([])
    for f_sol in data.get_fitting_solutions(p):
        #need to have the transformation in internal coordiantes
        t=f_sol.get_transformation()
        IMP.core.transform(rb,t)
        print "for ",i," Transformation is: ", rb.get_transformation()
        transformations[i].append(rb.get_transformation())
        IMP.core.transform(rb,t.get_inverse())
    states_of_particle.append(IMP.domino2.RigidBodyStates(transformations[i]))
    pst.set_particle_states(p,states_of_particle[i])
#set domino
print "here1"
domino_smp=IMP.domino2.DominoSampler(data.get_model(),pst)
print "here2"
domino_smp.set_maximum_score(50.)
print "here3"
domino_smp.show()
print "here4"
#get_sample currently fails
cs=domino_smp.get_sample()
print "here5"
# #    numbers.append(len(data.get_fitting_solutions(p)))
# #1.2 map states to particles
# #states=
# # ind=0
# # for i in range(data.get_number_of_components()):
# #     p=data.get_component(i)
# #     domino_smp.set_particle_states(p,states[ind:ind+numbers[i]])
# #     ind=ind+numbers[i]+1
# print "here"
# print "Found ", cs.get_number_of_configurations(), "solutions"
# lsc= IMP.domino2.Subset(ps)
for i,state in enumerate([[0,0,0,0],[0,1,2,3],[1,2,3,4]]):#10):#cs.get_number_of_configurations()):
    IMP.domino2.load_particle_states(IMP.domino2.Subset(pst.get_particles()),IMP.domino2.SubsetState(state),pst)
    #print the configuration:
    print "solution number:",i," scored:", mdl.evaluate(False)
    IMP.atom.write_pdb(IMP.atom.Hierarchies(ps),"sol_"+str(i)+".pdb")
