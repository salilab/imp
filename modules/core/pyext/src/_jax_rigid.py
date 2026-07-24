import numpy as np
import jax.tree_util
from dataclasses import dataclass
from IMP.algebra._jax_util import Transformation3D
import IMP


@jax.tree_util.register_dataclass
@dataclass
class _RigidBody:
    """Information on a single rigid body in the Model"""

    # Zero-based index of the body
    rb_index: int
    # Index of the corresponding IMP RigidBody particle in the IMP Model
    particle_index: int
    # Particle indexes of all members that are not themselves rigid bodies
    member_particle_indexes: jax.Array
    # Rigid body indexes of all members that are nested rigid bodies
    body_member_indexes: jax.Array
    # Rotation quaternion relative to parent rigid body for each nested body
    lquaternion: jax.Array

    def get_transformation(self, jm):
        """Get the transformation for this body's reference frame"""
        allrbs = jm['rigid_bodies']
        return Transformation3D(rotation=allrbs.quaternion[self.rb_index],
                                translation=jm['xyz'][self.particle_index])

    def get_internal_transformation(self, jm, i):
        """Get transformation for the ith nested rigid body, relative to
           this (parent) rigid body's reference frame."""
        allrbs = jm['rigid_bodies']
        child_body = allrbs.bodies[self.body_member_indexes[i]]
        return Transformation3D(
            rotation=self.lquaternion[i],
            translation=allrbs.intcoord[child_body.particle_index])

    def set_transformation_lazy(self, trans, jm):
        """Set the reference frame transformation from local to global
           coordinates, but do not change member global coordinates.
           Returns the new model."""
        allrbs = jm['rigid_bodies']
        allrbs.quaternion = allrbs.quaternion.at[self.rb_index].set(
            trans.rotation)
        jm['xyz'] = jm['xyz'].at[self.particle_index].set(trans.translation)
        return jm

    def update_members(self, jm):
        """Set the global coordinates for all members to match this body's
           reference frame. Returns the new model."""
        allrbs = jm['rigid_bodies']
        trans = self.get_transformation(jm).get_with_matrix()

        # Update global coordinates of non-body members
        intcoord = allrbs.intcoord[self.member_particle_indexes]
        jm['xyz'] = jm['xyz'].at[self.member_particle_indexes].set(
            trans.get_transformed(intcoord))

        # Update transformation of all nested rigid bodies
        for i in range(len(self.body_member_indexes)):
            body_index = self.body_member_indexes[i]
            jm = allrbs.bodies[body_index].set_transformation_lazy(
                trans * self.get_internal_transformation(jm, i), jm)
        return jm

    def set_transformation(self, trans, jm):
        """Set the reference frame transformation from local to global
           coordinates. This also sets the global coordinates for all
           members to match. Returns the new model."""
        jm = self.set_transformation_lazy(trans, jm)
        return self.update_members(jm)


@jax.tree_util.register_dataclass
@dataclass
class _AllRigidBodies:
    """Information on all rigid bodies in the Model"""

    # Internal coordinates indexed by particle index
    intcoord: jax.Array
    # Reference frame rotation quaternion indexed by rigid body index
    quaternion: jax.Array
    # Mapping from particle index to rigid body index
    rb_index_from_particle: dict
    # Particles that are non-rigid members of any rigid body
    # (these can change during sampling unlike rigid members)
    non_rigid_members: jax.Array
    # Information about each rigid body (as _RigidBody objects)
    bodies: list


_RB_LIST_KEY = IMP.ModelKey("rigid body list")
_RB_QUAT_KEY = IMP.Vector4DDerivKey("rigid_body_quaternion")
_RB_LQUAT_KEY = IMP.Vector4DDerivKey("rigid_body_local_quaternion")
_RB_IS_RIGID_KEY = IMP.IntKey("rigid_body__is_rigid")


def _get_rigid_body_indexes(m):
    """Get the particle indexes of all rigid bodies in the model"""
    assert m.get_has_data(_RB_LIST_KEY)
    rbl = m.get_data(_RB_LIST_KEY)
    rbl = IMP.SingletonContainer.get_from(rbl)
    return rbl.get_contents()


def _get_rigid_body_index(m, particle_index):
    """Given a particle index, return the corresponding rigid body index"""
    particle_from_rb_index = _get_rigid_body_indexes(m)
    rb_index_from_particle = {pi: rbi for (rbi, pi) in
                              enumerate(particle_from_rb_index)}
    return rb_index_from_particle[particle_index]


def _get_rigid_bodies(m):
    particle_from_rb_index = _get_rigid_body_indexes(m)
    rb_index_from_particle = {int(pi): rbi for (rbi, pi) in
                              enumerate(particle_from_rb_index)}
    intcoord = m.get_internal_coordinates_numpy()
    quaternion = m.get_numpy(_RB_LQUAT_KEY)[particle_from_rb_index]
    bodies = []
    for i, rb_ind in enumerate(particle_from_rb_index):
        rb = IMP.core.RigidBody(m, rb_ind)
        body_members = rb.get_body_member_particle_indexes()
        lquaternion = m.get_numpy(_RB_LQUAT_KEY)[body_members]
        bodies.append(_RigidBody(
            rb_index=i, particle_index=int(rb_ind),
            member_particle_indexes=rb.get_member_particle_indexes(),
            lquaternion=lquaternion,
            body_member_indexes=[rb_index_from_particle[i] for i in
                                 body_members]))
    is_rigid = m.get_numpy(_RB_IS_RIGID_KEY)
    return _AllRigidBodies(
        intcoord=intcoord, bodies=bodies,
        rb_index_from_particle=rb_index_from_particle,
        non_rigid_members=np.flatnonzero(is_rigid == 0),
        quaternion=quaternion)
