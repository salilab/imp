import IMP
import IMP.domino
import IMP.core
NUM_PARTICLES=6
NUM_STATES=40


#In this example we are looking for the positions of 6 particles that
#optimize a scoring function.

def run():
    mdl=IMP.Model()

    ####set up the particles
    ps=IMP.Particles()
    for i in range(NUM_PARTICLES):
        p=IMP.Particle(mdl)
        IMP.core.XYZR.setup_particle(p,IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0.,0.,0.),1.))
        ps.append(p)
        p.add_attribute(IMP.domino.node_name_key(),str(i))

    ####set up the discrete set of states
    ub=IMP.algebra.Vector3D(-10.,-10.,-10.)
    lb=IMP.algebra.Vector3D(10.,10.,10.)
    bb=IMP.algebra.BoundingBox3D(ub,lb)
    discrete_set=setup_discrete_sampling_space(ps,bb)

    ####add restraints (defining the scoring function)
    restraints=setup_restraints(ps)

    ####set the sampler
    sampler = IMP.domino.CartesianProductSampler(discrete_set,ps)
    #here alternative samplers can be PermutationSampler.

    ####optimize
    fast_enumerate(sampler,restraints,mdl)

#randomly sample NUM_STATES states for each particle
def setup_discrete_sampling_space(ps,bb):
    mdl=ps[0].get_model()
    atts=IMP.core.XYZ.get_xyz_keys()
    discrete_set = IMP.domino.MappedDiscreteSet(ps,atts)
    for p in ps:
        for i in range(NUM_STATES):
            #create a new state
            new_p=IMP.Particle(mdl)
            IMP.core.XYZ.setup_particle(new_p,IMP.algebra.random_vector_in_box(bb))
            print "adding state i:" + str(i)
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
        r.set_was_owned(True)
        rsrs.append(r)
        mdl.add_restraint(r)#different than doing mdl.add_restraint(rsrs) - Daniel ?
    return rsrs
def fast_enumerate(sampler,rsrs,mdl):
    """
    Enumerate the discrete sampling space to find the best combination
    """
    jt_filename = "six_particles.jt"
    jt = IMP.domino.JunctionTree()
    IMP.domino.read_junction_tree(jt_filename,jt)
    re = IMP.domino.RestraintEvaluator(sampler)
    d_opt = IMP.domino.DominoOptimizer(jt,mdl,re)
    #add restraints to be evaluates by the optimizer
    for r in rsrs:
        d_opt.add_restraint(r)
    #set the sampling space
    d_opt.set_sampling_space(sampler)
    d_opt.show_restraint_graph()
    print "here"
    d_opt.set_number_of_solutions(1)
    print "here1"
    d_opt.optimize(1)
    print "here2"

        # rg = d_opt = d_opt.get_graph()
        # scores=[6.5,7.05,7.63,7.85,8.04]
        # for i in range(num_sol):
        #     score_inf = rg.get_opt_combination(i).get_total_score()
        #     self.assertAlmostEqual(score_inf,scores[i],places=1)
if __name__=="__main__":
    run()
