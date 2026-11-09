"""Utility functions for JAX implementations of IMP.isd restraints."""

import jax
import jax.numpy as jnp
import jax.ops
import numpy


def _segmented_exclusive_cumprod(data, starts, reverse):
    """Exclusive cumulative product of data along axis 0, restarting at
       each position where `starts` is True. If `reverse` is True, the
       product is taken from the end of the array, and `starts` should
       flag the last element of each segment."""
    starts = starts.reshape(starts.shape + (1,) * (data.ndim - 1))

    def combine(a, b):
        aval, aflag = a
        bval, bflag = b
        return jnp.where(bflag, bval, aval * bval), aflag | bflag

    flags = jnp.broadcast_to(starts, data.shape)
    inclusive, _ = jax.lax.associative_scan(combine, (data, flags),
                                            reverse=reverse, axis=0)
    # Shift by one element to make the product exclusive, and start
    # each segment with 1
    ones = jnp.ones_like(data[:1])
    if reverse:
        shifted = jnp.concatenate((inclusive[1:], ones), axis=0)
    else:
        shifted = jnp.concatenate((ones, inclusive[:-1]), axis=0)
    return jnp.where(starts, 1., shifted)


def get_segment_prod(segment_ids, num_segments):
    """Get a differentiable function equivalent to jax.ops.segment_prod.

       jax.ops.segment_prod cannot be differentiated if segment IDs are
       repeated. The returned function takes a single `data` argument
       and computes the product of `data` along axis 0 for each segment,
       given the fixed (sorted) `segment_ids`. Unlike a derivative obtained
       by dividing the product by each element, this handles elements
       that are zero.

       @param segment_ids A sorted NumPy array of segment IDs, one for
              each element of axis 0 of `data`.
       @param num_segments The number of segments.
    """
    segment_ids = numpy.asarray(segment_ids)
    # Flag the first and last element of each segment
    starts = numpy.ones(len(segment_ids), dtype=bool)
    starts[1:] = segment_ids[1:] != segment_ids[:-1]
    ends = numpy.ones(len(segment_ids), dtype=bool)
    ends[:-1] = starts[1:]

    @jax.custom_jvp
    def segment_prod(data):
        return jax.ops.segment_prod(data, segment_ids, num_segments,
                                    indices_are_sorted=True)

    @segment_prod.defjvp
    def segment_prod_jvp(primals, tangents):
        data, = primals
        data_dot, = tangents
        # The derivative of each segment's product with respect to one
        # element is the product of all other elements in that segment
        others = (_segmented_exclusive_cumprod(data, starts, reverse=False)
                  * _segmented_exclusive_cumprod(data, ends, reverse=True))
        out_dot = jax.ops.segment_sum(others * data_dot, segment_ids,
                                      num_segments, indices_are_sorted=True)
        return segment_prod(data), out_dot

    return segment_prod
