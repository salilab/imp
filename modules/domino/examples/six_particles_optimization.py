import IMP
import IMP.domino
import IMP.core
NUM_PARTICLES=6
NUM_STATES=40

#randomly sample NUM_STATES states for each particle
def setup_discrete_sampling_space(ps,bb):
    mdl=ps[0].get_model()
    atts=IMP.core.XYZ.get_xyz_keys()
    ps_cont = IMP.container.ListSingletonContainer(ps)
    discrete_set = IMP.domino.MappedDiscreteSet(ps_cont,atts)
    for p in ps:
        for i in range(NUM_STATES):
            #create a new state
            new_p=IMP.Particle(mdl)
            IMP.core.XYZ.setup_particle(new_p,IMP.algebra.get_random_vector_in(bb))
            discrete_set.add_state(new_p)
            discrete_set.add_mapped_state(p,new_p)
    return discrete_set

#set restraints
def setup_restraints(ps):
    mdl=ps[0].get_model()
    rsrs=[]
    pairs=[[0,1],[0,2],[1,2],[2,3],[3,4],[4,5],[3,5]]
    sf = IMP.core.Harmonic(1.0, 0.5)
    for pair in pairs:
        r=IMP.core.DistanceRestraint(sf, ps[pair[0]], ps[pair[1]])
        r.set_was_used(True)
        rsrs.append(r)
        mdl.add_restraint(r)#different than doing mdl.add_restraint(rsrs) - Daniel ?
    return rsrs
def fast_enumerate(sampler,rsrs,mdl,num_sols):
    """
    Enumerate the discrete sampling space to find the best combination
    """
    jt_filename = IMP.domino.get_example_path("six_particles.jt")
    jt = IMP.domino.JunctionTree()
    IMP.domino.read_junction_tree(jt_filename,jt)
    re = IMP.domino.RestraintEvaluator(sampler)
    d_opt = IMP.domino.DominoOptimizer(jt,mdl,re)
    #add restraints to be evaluates by the optimizer
    for r in rsrs:
        d_opt.add_restraint(r)
    #set the sampling space
    d_opt.set_sampling_space(sampler)
    #if you wish to see how the graph looks like remove this
    #line from comment
    #d_opt.show_restraint_graph()
    d_opt.set_number_of_solutions(num_sols)
    d_opt.optimize(num_sols)
    return d_opt

#### Example main code ####
IMP.set_log_level(IMP.SILENT)
mdl=IMP.Model()

#1. set up the particles
print "setting up particles"
ps=IMP.Particles()
for i in range(NUM_PARTICLES):
    p=IMP.Particle(mdl)
    IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0.,0.,0.),1.))
    ps.append(p)
    p.add_attribute(IMP.domino.node_name_key(),str(i))

#2. set up the discrete set of states
print "setting up a discrete set of states"
ub=IMP.algebra.Vector3D(-10.,-10.,-10.)
lb=IMP.algebra.Vector3D(10.,10.,10.)
bb=IMP.algebra.BoundingBox3D(ub,lb)
discrete_set=setup_discrete_sampling_space(ps,bb)

#3. add restraints (defining the scoring function)
print "setting up restraints"
restraints=setup_restraints(ps)

#4. set the sampler
print "setting up sampling"
sampler = IMP.domino.CartesianProductSampler(discrete_set,ps)
#alternative samplers can be PermutationSampler.

#5. optimize
print "optimizing"
num_sols=5
d_opt=fast_enumerate(sampler,restraints,mdl,num_sols)

#6. print results
rg = d_opt.get_graph()
for i in range(num_sols):
    sol = rg.get_opt_combination(i)
    print "solution number:",i," is: ", sol.key() ," with score:", sol.get_total_score()
