"""@namespace IMP.jax
   @brief Support for the JAX Python library.

   IMP currently has rudimentary support for running on a graphics
   processing unit (GPU) or similar systems such as
   Tensor Processing Units (TPUs). This support uses the
   [JAX](https://docs.jax.dev/) Python library.
"""

import numpy as np
import jax.numpy as jnp
import jax.random
import jax.tree_util
import IMP


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


def get_random_key():
    """Get a new JAX random key seeded from IMP's random number generator"""
    return jax.random.key(IMP.random_number_generator())


class _Remap:
    """A mapping from full particle indexes to and from the compact array.
       This is used in _CompactArray, below. It is hashed by identity so
       that it (unlike the arrays it contains) can be used as pytree aux data.

       @param full_view A NumPy view of the entire IMP Model.
       @param indexes A NumPy array of indexes of the particles that
              have the attribute.
       @param mapping A NumPy array that maps original Particle indexes
              to indexes into the `data` array.
       """
    def __init__(self, full_view, indexes, mapping):
        self.full_view, self.indexes = full_view, indexes
        self.mapping = mapping


@jax.tree_util.register_pytree_node_class
class _CompactArray:
    """Access an IMP Model Float attribute as a compacted or sparse array.
       An IMP attribute array (returned by Model.get_numpy()) can be sparsely
       populated. Compact it down to a flat array of only the particles that
       have the attribute. Original Particle indexes are mapped to indexes
       into the compacted array at JAX trace time. This should make JAX code
       more performant since only the compacted array needs to be transferred
       to and from the GPU.

       @param data Compact array of only the used particles.
       @param remap A _Remap object that maps full particle indexes to and
              from the compact array.
       """

    def __init__(self, data, remap):
        self.data, self.remap = data, remap

    @classmethod
    def from_model(cls, m, fk):
        """Create a new CompactArray for the given FloatKey `fk` in the
           given IMP Model `m`."""
        full_view = m.get_numpy(fk)
        # IMP uses infinity to represent particles without the attribute
        indexes = np.nonzero(full_view != np.inf)[0]
        # Any particle not in `indexes` is mapped to the last element,
        # which is inf (just as in the original full view)
        mapping = np.full(len(full_view), len(full_view) + 1, dtype=np.int32)
        mapping[indexes] = np.arange(len(indexes), dtype=np.int32)
        remap = _Remap(full_view, indexes, mapping)
        return cls(np.concatenate((full_view[indexes], np.array([np.inf]))),
                   remap)

    def tree_flatten(self):
        # Convert to JAX. Only `data` is sent to the device; everything
        # else is static (aux data)
        return (self.data,), self.remap

    @classmethod
    def tree_unflatten(cls, aux_data, children):
        return cls(data=children[0], remap=aux_data)

    def _map(self, idx):
        return self.remap.mapping[idx]

    def __getitem__(self, idx):
        """Lookup by particle index"""
        return self.data[self._map(idx)]

    @property
    def at(self):
        """Like jax.Array.at; allow in-place array modification"""
        return _AtCompactArray(self)

    def sync(self):
        """Copy the JAX data back to the IMP Model"""
        self.remap.full_view[self.remap.indexes] = self.data[:-1]


class _AtCompactArray:
    """Helper class for _CompactArray.at"""
    def __init__(self, arr):
        self.arr = arr

    def __getitem__(self, idx):
        return _AtIdxCompactArray(self.arr, self.arr._map(idx))


class _AtIdxCompactArray:
    """Helper class for _CompactArray.at[idx]"""
    def __init__(self, arr, rows):
        self.arr, self.rows = arr, rows

    def _new(self, data):
        return _CompactArray(data, self.arr.remap)

    def set(self, v):
        return self._new(self.arr.data.at[self.rows].set(v))

    def add(self, v):
        return self._new(self.arr.data.at[self.rows].add(v))
