import IMP.atom
import IMP.core
import IMP.helper
import random,math

def read_components(fns,mdl):
    mhs = IMP.atom.Hierarchies()
    sel = IMP.atom.CAlphaPDBSelector()
    for fn in fns:
        mhs.append(IMP.atom.read_pdb(fn,mdl,sel))
        IMP.atom.add_radii(mhs[-1])
    return mhs

def set_rigid_bodies(mhs,mdl):
    return IMP.helper.set_rigid_bodies(mhs)
def set_colision_detection_between_rigid_bodies(rb1,rb2):
    rbs=IMP.core.RigidBodies()
    rbs.append(rb1)
    rbs.append(rb2)
    simple_r = IMP.helper.create_simple_excluded_volume_on_rigid_bodies(rbs)
    r = simple_r.get_restraint()
    print r
    rb1.get_model().add_restraint(r)
    return r

def create_random_transformation(angle_step=30.,translation_step=4.):
    zero_vec=IMP.algebra.Vector3D(0.,0.,0.)
    z_vec=IMP.algebra.Vector3D(0.,0.,1.)
    rand_t=IMP.algebra.Transformation3D(
        IMP.algebra.get_rotation_in_radians_about_axis(
        z_vec,
        random.uniform(-math.radians(angle_step),math.radians(angle_step))),
        IMP.algebra.get_random_vector_in(IMP.algebra.Sphere3D(zero_vec,translation_step)))
    return rand_t
