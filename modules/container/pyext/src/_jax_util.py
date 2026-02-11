import functools
import jax.numpy as jnp


def _get_static_container_contents(c):
    """Get the contents of a container. Require that they not change."""
    # StaticListContainer has no Python implementation, so get_derived_object()
    # will fail. But as the name implies, the contents are static, so we
    # can just use the base class:
    if c.get_type_name() == 'StaticListContainer':
        return c.get_contents()
    else:
        return c.get_derived_object()._get_static_contents()


def _get_jax_container_restraint(r):
    """Return a JAX implementation for SingletonsRestraint,
       PairsRestraint, etc."""
    indexes = _get_static_container_contents(r.get_container())
    score = r.get_score_object().get_derived_object()
    ji = score._get_jax(r.get_model(), indexes)
    score_jax = ji.score_func

    def jax_restraint(jm):
        return jnp.sum(score_jax(jm))
    return r._wrap_jax(jax_restraint, keys=ji._keys)


def _get_jax_container_constraint(r):
    """Return a JAX implementation for SingletonsConstraint,
       PairsConstraint, etc."""
    container = r.get_container().get_derived_object()
    mod = r.get_before_modifier().get_derived_object()
    ji = mod._get_jax(r.get_model(), index=None)
    indexes = container._get_static_contents()
    return r._wrap_jax(
        functools.partial(ji.apply_func, indexes=indexes),
        keys=ji._keys)
