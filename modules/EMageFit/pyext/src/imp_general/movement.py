"""@namespace IMP.EMageFit.imp_general.movement
   Utility functions to handle movement.
"""

import IMP
import IMP.algebra
import IMP.atom
import IMP.core
import logging
import random


def apply_random_transform(rb, max_trans=100):
    """
        Apply a random transformation to the rigid body and change the reference
        frame
    """
    bb = IMP.algebra.BoundingBox3D(
                  IMP.algebra.Vector3D(-max_trans, -max_trans, -max_trans),
                  IMP.algebra.Vector3D(max_trans, max_trans, max_trans))
    Trand = IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                         IMP.algebra.get_random_vector_in(bb))
    ref = rb.get_reference_frame()
    Tr = ref.get_transformation_to()
    T = IMP.algebra.compose(Trand, Tr)
    rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(T))
    return Trand


def apply_transformation_to_hierarchy(prot, T, fn_write=False):
    """
        If fn_write is different from False, write to file
    """
    R = T.get_rotation()
    t = T.get_translation()
    xyz1 = [IMP.core.XYZ(l) for l in IMP.atom.get_leaves(prot)]
    coords = [p.get_coordinates() for p in xyz1]
    newvs = [R.get_rotated(v) + t for v in coords]
    for i in range(len(newvs)):
        xyz1[i].set_coordinates(newvs[i])
    if(fn_write):
        IMP.atom.write_pdb(prot, fn_write)


def get_random_transformation(max_distance, max_angle, seed=-1):
    """
        Return a random transformation
        @param max_distance Maximum translation allowed
        @param max_angle Maximum rotation angle allowed
        @param seed If specified, seed for random number generator
    """

    if seed == -1:
        random.seed()
    else:
        random.seed(seed)

    phi = random.uniform(-max_angle, max_angle)
    theta = random.uniform(-max_angle, max_angle)
    psi = random.uniform(-max_angle, max_angle)
    trans_x = random.uniform(-max_distance, max_distance)
    trans_y = random.uniform(-max_distance, max_distance)
    trans_z = random.uniform(-max_distance, max_distance)
    trns = IMP.algebra.Vector3D(trans_x, trans_y, trans_z)
    rot = IMP.algebra.get_rotation_from_fixed_zyz(phi, theta, psi)
    transformation = IMP.algebra.Transformation3D(rot, trns)
    return transformation
