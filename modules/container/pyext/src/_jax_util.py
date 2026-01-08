import jax.numpy as jnp


def _get_jax_container_restraint(r):
    """Return a JAX implementation for SingletonsRestraint,
       PairsRestraint, etc."""
    container = r.get_container().get_derived_object()
    score = r.get_score_object().get_derived_object()
    ji = score._get_jax()
    score_jax = ji.score_func
    indexes = container._get_static_contents()

    def jax_restraint(X):
        return jnp.sum(score_jax(X, indexes))
    return r._wrap_jax(jax_restraint, keys=ji._keys)
