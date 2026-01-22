%extend IMP::example::ExampleConstraint {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the constraint using JAX"""
        import functools
        def apply_func(jm, key, index):
            jm[key] = jm[key].at[index].add(1)
            return jm
        key = self.get_key()
        f = functools.partial(apply_func, key=key.get_string(),
                              index=self.get_index())
        return self._wrap_jax(f, keys=(key,))
  %}
}

%extend IMP::example::ExampleSingletonModifier {
  %pythoncode %{
    def _get_jax(self, m, index=None):
        """Implementation of the modifier using JAX"""
        import functools
        import jax.numpy as jnp
        def apply_func(jm, indexes, box):
            jm['xyz'] = jm['xyz'].at[index].set(
                jnp.mod(jm['xyz'][index], box[1] - box[0]) + box[0])
            return jm
        box = jnp.array([self.get_bounding_box().get_corner(x)
                         for x in range(2)])
        return self._wrap_jax(
            functools.partial(apply_func, box=box))
  %}
}

%extend IMP::example::ExamplePairScore {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the score using JAX"""
        import jax.numpy as jnp
        import functools
        def pair_score(jm, indexes, x0, k):
            xyzs = jm['xyz'][indexes]
            diff = jnp.linalg.norm(xyzs[:,0] - xyzs[:,1], axis=1) - x0
            return 0.5 * k * diff * diff
        f = functools.partial(pair_score, x0=self.get_mean(),
                              k=self.get_force_constant())
        return self._wrap_jax(f)
  %}
}

%extend IMP::example::ExampleRestraint {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the restraint using JAX"""
        import functools
        def jax_restraint(jm, k, pi):
            xyz = jm['xyz'][pi]
            return 0.5 * k * xyz[2] * xyz[2]
        f = functools.partial(jax_restraint, k=self.get_force_constant(),
                              pi=self.get_index())
        return self._wrap_jax(f)
  %}
}

%extend IMP::example::ExampleUnaryFunction {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the unary function using JA"""
        import functools
        def score(feature, center, k):
            return 0.5 * k * (feature - center) ** 2
        return functools.partial(score, center=self.get_center(),
                                 k=self.get_force_constant())
  %}
}
