import numpy as np
import jax.numpy as jnp
import jax.tree_util
from dataclasses import dataclass
from IMP.algebra._jax_util import Transformation3D
import IMP


@dataclass
class _NestedRigidBodies:
    """Information on all nested bodies for a given rigid body.
       This is stored in a class so that it can be used as JAX pytree
       aux data, since this information is static and does not change
       during a simulation. The mapping from IMP particle indexes to
       the more compact per-rigid body arrays (e.g. quaternion) or
       per-nested-rigid body arrays (e.g. lquaternion) is done at jit time.
    """
    # Rigid body indexes of all members that are nested rigid bodies
    rb_indexes: jax.Array
    # Nested rigid body indexes of all members that are nested rigid bodies
    nrb_indexes: jax.Array

    def __len__(self):
        """Get the number of nested rigid bodies"""
        return len(self.rb_indexes)

    def get_nth(self, allrbs, i):
        """Get the RigidBody object for the ith nested body, as well as
           the index into per-nested-rigid body arrays."""
        body = allrbs.bodies[self.rb_indexes[i]]
        nrb_index = self.nrb_indexes[i]
        return body, nrb_index


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
    # All members that are nested rigid bodies (static)
    nested_bodies: _NestedRigidBodies = jax.tree.static()

    def get_transformation(self, jm):
        """Get the transformation for this body's reference frame"""
        allrbs = jm['rigid_bodies']
        return Transformation3D(rotation=allrbs.quaternion[self.rb_index],
                                translation=jm['xyz'][self.particle_index])

    def get_internal_transformation(self, jm, i):
        """Get transformation for the ith nested rigid body, relative to
           this (parent) rigid body's reference frame."""
        allrbs = jm['rigid_bodies']
        child_body, nrb_index = self.nested_bodies.get_nth(allrbs, i)
        return Transformation3D(
            rotation=allrbs.lquaternion[nrb_index],
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
        for i in range(len(self.nested_bodies)):
            body, nrb_index = self.nested_bodies.get_nth(allrbs, i)
            jm = body.set_transformation_lazy(
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
    # Rotation quaternion relative to parent rigid body for each nested body
    lquaternion: jax.Array
    # Mapping from nested rigid body index to particle index
    particle_from_nrb_index: jax.Array
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


def _get_nested_rigid_body_indexes(m, rigid_body_indexes):
    """Get the particle indexes of all nested rigid bodies in the model"""
    for pi in rigid_body_indexes:
        # A nested rigid body is itself a member of another body
        if IMP.core.RigidBodyMember.get_is_setup(m, pi):
            yield pi


def _get_rigid_bodies(m):
    particle_from_rb_index = _get_rigid_body_indexes(m)
    rb_index_from_particle = {int(pi): rbi for (rbi, pi) in
                              enumerate(particle_from_rb_index)}
    particle_from_nrb_index = list(_get_nested_rigid_body_indexes(
        m, particle_from_rb_index))
    nrb_index_from_particle = {int(pi): rbi for (rbi, pi) in
                               enumerate(particle_from_nrb_index)}
    intcoord = m.get_internal_coordinates_numpy()
    quaternion = jnp.asarray(m.get_numpy(_RB_QUAT_KEY)[particle_from_rb_index])
    lquaternion = jnp.asarray(
        m.get_numpy(_RB_LQUAT_KEY)[particle_from_nrb_index])
    bodies = []
    for i, rb_ind in enumerate(particle_from_rb_index):
        rb = IMP.core.RigidBody(m, rb_ind)
        body_members = rb.get_body_member_particle_indexes()
        nrb = _NestedRigidBodies(
            rb_indexes=[rb_index_from_particle[i] for i in body_members],
            nrb_indexes=[nrb_index_from_particle[i] for i in body_members])
        bodies.append(_RigidBody(
            rb_index=i, particle_index=int(rb_ind),
            member_particle_indexes=rb.get_member_particle_indexes(),
            nested_bodies=nrb))
    is_rigid = m.get_numpy(_RB_IS_RIGID_KEY)
    return _AllRigidBodies(
        intcoord=intcoord, bodies=bodies,
        rb_index_from_particle=rb_index_from_particle,
        particle_from_nrb_index=particle_from_nrb_index,
        non_rigid_members=np.flatnonzero(is_rigid == 0),
        quaternion=quaternion,
        lquaternion=lquaternion)
