import math
import jax.lax
import jax.numpy as jnp
import jax.random

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
