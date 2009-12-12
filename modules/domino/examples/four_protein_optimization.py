import IMP
import IMP.domino
import IMP.core
import IMP.helper
import random,math
NUM_STATES=5


#In this example we are looking for the positions of 4 proteins that
#optimize a scoring function.


def create_random_transformation(angle_step=30.,translation_step=4.):
    zero_vec=IMP.algebra.Vector3D(0.,0.,0.)
    z_vec=IMP.algebra.Vector3D(0.,0.,1.)
    rand_t=IMP.algebra.Transformation3D(
        IMP.algebra.rotation_in_radians_about_axis(
        z_vec,
        random.uniform(-math.radians(angle_step),math.radians(angle_step))),
        IMP.algebra.random_vector_in_sphere(zero_vec,translation_step))
    return rand_t


def run():
    IMP.set_log_level(IMP.TERSE)
    mdl=IMP.Model()
    sel=IMP.atom.CAlphaSelector()
    ####set up the particles
    ps=IMP.Particles()
    mhs=IMP.atom.Hierarchies()
    for prot_name,chain_id in [["1z5s_A.pdb","A"],["1z5s_B.pdb","B"],["1z5s_C.pdb","C"],["1z5s_D.pdb","D"]]:
        mhs.append(IMP.atom.read_pdb(IMP.domino.get_example_path(prot_name),
                                     mdl,sel))
        mhs[-1].get_particle().add_attribute(IMP.domino.node_name_key(),chain_id)
        IMP.atom.add_radii(mhs[-1])
        ps.append(mhs[-1].get_particle())
    rbs=IMP.helper.set_rigid_bodies(mhs)

    ####set up the discrete set of states
    discrete_set=setup_discrete_sampling_space(ps)

    ####add restraints (defining the scoring function)
    restraints=setup_restraints(rbs)

    ####set the sampler
    sampler = IMP.domino.CartesianProductSampler(discrete_set,ps)
    #here alternative samplers can be PermutationSampler.

    ####optimize
    fast_enumerate(sampler,restraints,mdl)

#randomly sample NUM_STATES states for each particle
def setup_discrete_sampling_space(ps):
    mdl=ps[0].get_model()
    atts=IMP.core.XYZ.get_xyz_keys()
    discrete_set = IMP.domino.MappedDiscreteSet(ps,atts)
    for p in ps:
        for i in range(NUM_STATES):
            #create a new state
            rand_t=create_random_transformation()
            new_p=IMP.Particle(mdl)
            IMP.domino.Transformation.setup_particle(new_p,rand_t)
            discrete_set.add_state(new_p)
            discrete_set.add_mapped_state(p,new_p)
    return discrete_set

#set restraints
def setup_restraints(rbs):
    mdl=rbs[0].get_particle().get_model()
    rsrs=[]
    pairs=[[0,1],[0,2],[0,3],[1,3]]
    for [i,j] in pairs:
        my_rbs=IMP.core.RigidBodies()
        my_ps=IMP.Particles()
        my_ps.append(rbs[i].get_particle())
        my_ps.append(rbs[j].get_particle())
        my_rbs.append(rbs[i])
        my_rbs.append(rbs[j])
        rsrs.append([IMP.helper.create_simple_excluded_volume_on_rigid_bodies(my_rbs),my_ps])
    return rsrs

def fast_enumerate(sampler,rsrs,mdl):
    """
    Enumerate the discrete sampling space to find the best combination
    """
    jt_filename = "1z5s.jt"
    jt = IMP.domino.JunctionTree()
    IMP.domino.read_junction_tree(jt_filename,jt)
    re = IMP.domino.RestraintEvaluator(sampler)
    d_opt = IMP.domino.DominoOptimizer(jt,mdl,re)
    #add restraints to be evaluates by the optimizer
    for r in rsrs:
        d_opt.add_restraint(r[0].get_restraint(),r[1])
    #set the sampling space
    d_opt.set_sampling_space(sampler)
    d_opt.show_restraint_graph()
    d_opt.set_number_of_solutions(1)
    d_opt.optimize(1)

        # rg = d_opt = d_opt.get_graph()
        # scores=[6.5,7.05,7.63,7.85,8.04]
        # for i in range(num_sol):
        #     score_inf = rg.get_opt_combination(i).get_total_score()
        #     self.assertAlmostEqual(score_inf,scores[i],places=1)
if __name__=="__main__":
    run()
