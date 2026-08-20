%pythonbegin %{
  import functools
%}

%extend IMP::isd::UniformPrior {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        indexes = jnp.array([self.get_index()])
        def score(jm, lb, ub, k):
            nuisance = jm['nuisance'][indexes]
            return 0.5 * k * (nuisance - jnp.clip(nuisance, lb, ub)) ** 2
        rng = self.get_range()
        f = functools.partial(score, lb=rng[0], ub=rng[1], k=self.get_k())
        return self._wrap_jax(f, keys=[Scale.get_scale_key()])
  %}
}

%extend IMP::isd::JeffreysRestraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        indexes = jnp.array([self.get_index()])
        def score(jm):
            nuisance = jm['nuisance'][indexes]
            return jnp.sum(jnp.log(nuisance))
        return self._wrap_jax(score, keys=[Scale.get_scale_key()])
  %}
}

%extend IMP::isd::LogWrapper {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        funcs, keys = self._get_restraint_jax_funcs_keys()
        def jax_sf(jm):
            scores = jnp.asarray([f(jm) for f in funcs])
            return -jnp.log(jnp.prod(scores))
        return self._wrap_jax(jax_sf, keys=keys)
  %}
}

%extend IMP::isd::NuisanceScoreState {
  %pythoncode %{
    def _get_jax(self):
        import jax.lax
        import math
        import jax.numpy as jnp
        def apply_func(jm, index, upper_f, upper_p, lower_f, lower_p):
            nuisances = jm['nuisance']
            up = jm['upper'][index] if upper_f else math.inf
            if upper_p:
                up = jax.lax.min(up, nuisances[jm['p_upper'][index]])
            low = jm['lower'][index] if lower_f else -math.inf
            if lower_p:
                low = jax.lax.max(low, nuisances[jm['p_lower'][index]])
            jm['nuisance'] = jm['nuisance'].at[index].set(
                    jnp.clip(nuisances[index], low, up))
            return jm
        p = self.get_index()
        m = self.get_model()
        n = Nuisance(m, p)
        keys = [n.get_nuisance_key()]
        upper_f, upper_p = n.get_has_upper_float(), n.get_has_upper_particle()
        lower_f, lower_p = n.get_has_lower_float(), n.get_has_lower_particle()
        if upper_f:
            keys.append(n.get_upper_key())
        if upper_p:
            keys.append(n.get_upper_particle_key())
        if lower_f:
            keys.append(n.get_lower_key())
        if lower_p:
            keys.append(n.get_lower_particle_key())
        f = functools.partial(apply_func, index=p, upper_f=upper_f,
                              upper_p=upper_p, lower_f=lower_f,
                              lower_p=lower_p)
        return self._wrap_jax(f, keys)
  %}
}
