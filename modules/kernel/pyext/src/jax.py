"""@namespace IMP.jax
   @brief Support for the JAX Python library.

   IMP currently has rudimentary support for running on a graphics
   processing unit (GPU) or similar systems such as
   Tensor Processing Units (TPUs). This support uses the
   [JAX](https://docs.jax.dev/) Python library.
"""

import jax.numpy as jnp


class Space:
    """The space in which restraints are evaluated. See FreeSpace for the
       default unbounded space, or PeriodicSpace for a space that implements
       periodic boundary conditions."""

    def distance(dr):
        """If given an array of particle-particle vectors, return an array
           of distances. If given a single particle-particle vector, return
           a single distance."""
        pass

    def shift(r, dr):
        """Shift r by dr and return new r"""
        pass

    def shift_indexes(r, indexes, dr):
        """Modify r[indexes] in place by adding dr"""
        pass


class FreeSpace(Space):
    """An unbounded space with no periodic boundary conditions."""

    @staticmethod
    def distance(dr):
        return jnp.linalg.norm(dr, axis=-1)

    @staticmethod
    def shift(r, dr):
        return r + dr

    @staticmethod
    def shift_indexes(r, indexes, dr):
        return r.at[indexes].add(dr)


class PeriodicSpace(Space):
    """A space with periodic boundary conditions.

       @param side A 3D vector of the periodic boundary box dimensions.
    """

    def __init__(self, side):
        self.side = jnp.asarray(side)

    def distance(self, dr):
        p_dr = jnp.mod(dr + self.side * 0.5, self.side) - 0.5 * self.side
        return jnp.linalg.norm(p_dr, axis=-1)

    def shift(self, r, dr):
        return jnp.mod(r + dr, self.side)

    def shift_indexes(self, r, indexes, dr):
        newr = jnp.mod(r[indexes] + dr, self.side)
        return r.at[indexes].set(newr)
