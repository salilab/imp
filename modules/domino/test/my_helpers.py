import IMP.atom
import IMP.core
import IMP.helper
import random,math

def read_components(fns,mdl):
    mhs = IMP.atom.Hierarchies()
    sel = IMP.atom.CAlphaSelector()
    for fn in fns:
        mhs.append(IMP.atom.read_pdb(fn,mdl,sel))
        IMP.atom.add_radii(mhs[-1])
    return mhs

def set_rigid_bodies(mhs,mdl):
    rbs = IMP.Particles()
    for i,mh in enumerate(mhs):
        rb_p = IMP.Particle(mdl)
        rb_state = IMP.helper.create_rigid_body(rb_p,
                       IMP.core.XYZs(IMP.core.get_leaves(mh)))
        #mdl.add_score_state(rb_state)
        IMP.helper.create_cover(rb_p,IMP.core.RigidMembersRefiner())
        rbs.append(rb_p)
    return rbs
def set_colision_detection_between_rigid_bodies(rb1,rb2):
    mdl=rb1.get_model()
    lsc= IMP.core.ListSingletonContainer()
    lsc.add_particle(rb1)
    lsc.add_particle(rb2)
    #Set up the nonbonded list
    nbl= IMP.core.ClosePairsScoreState(lsc)
    #bacause you want to restraint the actual rigid bodies and not the its particles
    nbl.set_close_pairs_finder(IMP.core.RigidClosePairsFinder())
    mdl.add_score_state(nbl);
    #Set the amount particles need to move before the list is updated
    #nbl.set_slack(2.0);
    nbl.set_slack(2.0);
    #Set up excluded volume
    pss = IMP.core.SphereDistancePairScore(IMP.core.HarmonicLowerBound(0,1))
    evr = IMP.core.PairsRestraint(pss, nbl.get_close_pairs_container())
    mdl.add_restraint(evr)
    evr.evaluate(None)
    return evr

def create_random_transformation(angle_step=30.,translation_step=4.):
    zero_vec=IMP.algebra.Vector3D(0.,0.,0.)
    z_vec=IMP.algebra.Vector3D(0.,0.,1.)
    rand_t=IMP.algebra.Transformation3D(
        IMP.algebra.rotation_in_radians_about_axis(
        z_vec,
        random.uniform(-math.radians(angle_step),math.radians(angle_step))),
        IMP.algebra.random_vector_in_sphere(zero_vec,translation_step))
    return rand_t
