import math
import jax.lax
import jax.numpy as jnp
import jax.random
from dataclasses import dataclass


def get_random_vector_in_3d_sphere(key, radius, shape=1):
    """Return a random vector uniformly distributed in a 3-dimensional
       sphere of given radius at the origin"""
    k1, k2, k3 = jax.random.split(key, 3)
    phi = jax.random.uniform(k1, shape=shape,
                             minval=0., maxval=2. * math.pi)
    theta = jnp.acos(jax.random.uniform(k2, shape=shape,
                                        minval=-1., maxval=1.))
    r = radius * jnp.cbrt(jax.random.uniform(k3, shape=shape,
                                             minval=0.0, maxval=1.0))
    return jnp.array([r * jnp.sin(theta) * jnp.cos(phi),
                      r * jnp.sin(theta) * jnp.sin(phi),
                      r * jnp.cos(theta)]).T


def get_random_vector_in_sphere(key, n, radius):
    """Return a random vector uniformly distributed in an n-dimensional
       sphere of given radius at the origin"""
    # Simply sample vectors in a cube, and discard any that fall outside
    # of the sphere:
    box_lbound = jnp.full(n, -radius)
    box_ubound = jnp.full(n, radius)

    def vector_out_radius(kv):
        key, vector = kv
        return jnp.dot(vector, vector) > radius * radius

    def sample_vector(kv):
        key, vector = kv
        key, subkey = jax.random.split(key)
        return (key, jax.random.uniform(subkey, shape=n, minval=box_lbound,
                                        maxval=box_ubound))

    if n == 3:
        return get_random_vector_in_3d_sphere(key, radius)
    else:
        return jax.lax.while_loop(vector_out_radius, sample_vector,
                                  (key, box_ubound * 2.))[1]


def get_random_vector_on_3d_sphere(key, radius, shape=1):
    """Return a random vector uniformly distributed on the surface of
       a 3-dimensional sphere of given radius at the origin"""
    k1, k2 = jax.random.split(key, 2)
    phi = jax.random.uniform(k1, shape=shape,
                             minval=0., maxval=2. * math.pi)
    theta = jnp.acos(jax.random.uniform(k2, shape=shape,
                                        minval=-1., maxval=1.))
    return jnp.array([radius * jnp.sin(theta) * jnp.cos(phi),
                      radius * jnp.sin(theta) * jnp.sin(phi),
                      radius * jnp.cos(theta)]).T


def _quaternion_to_rotation_matrix(quaternion):
    """Convert (normalized) quaternion to rotation matrix"""
    v0 = quaternion[0]
    v1 = quaternion[1]
    v2 = quaternion[2]
    v3 = quaternion[3]
    v0s = v0**2
    v1s = v1**2
    v2s = v2**2
    v3s = v3**2
    v12 = v1 * v2
    v01 = v0 * v1
    v02 = v0 * v2
    v23 = v2 * v3
    v03 = v0 * v3
    v13 = v1 * v3
    return jnp.array(
        [[v0s + v1s - v2s - v3s, 2. * (v12 + v03), 2. * (v13 - v02)],
         [2. * (v12 - v03), v0s - v1s + v2s - v3s, 2. * (v23 + v01)],
         [2. * (v13 + v02), 2. * (v23 - v01), v0s - v1s - v2s + v3s]])


def _quaternion_multiply(q1, q2):
    """Multiply two quaternions and return the result"""
    return jnp.array(
        [q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3],
         q1[0] * q2[1] + q1[1] * q2[0] + q1[2] * q2[3] - q1[3] * q2[2],
         q1[0] * q2[2] - q1[1] * q2[3] + q1[2] * q2[0] + q1[3] * q2[1],
         q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1] + q1[3] * q2[0]])


@jax.tree_util.register_dataclass
@dataclass(frozen=True)
class Transformation3DWithMatrix:
    """A transformation that includes the rotation matrix. Usually these
       are created using Transformation3D.get_with_matrix(). These can be
       composed with other transformations via multiplication."""

    # Rotation (as a quaternion)
    rotation: jax.Array
    # Rotation matrix (3x3 array)
    rotation_matrix: jax.Array
    # Translation vector
    translation: jax.Array

    def get_transformed(self, v):
        """Transform and return all vectors v"""
        return jnp.vecmat(v, self.rotation_matrix) + self.translation

    def __mul__(self, other):
        # Compose with another transformation
        return Transformation3D(
            rotation=_quaternion_multiply(self.rotation, other.rotation),
            translation=self.get_transformed(other.translation))


@jax.tree_util.register_dataclass
@dataclass
class Transformation3D:
    """A 3D transformation as a rotation quaternion plus translation."""
    # Rotation (as a quaternion)
    rotation: jax.Array
    # Translation vector
    translation: jax.Array

    def get_with_matrix(self):
        """Calculate the 3x3 rotation matrix and return a new object that
           includes it. This object can then be used to transform vectors
           and can be composed with other transformations."""
        return Transformation3DWithMatrix(
            rotation=self.rotation, translation=self.translation,
            rotation_matrix=_quaternion_to_rotation_matrix(self.rotation))
