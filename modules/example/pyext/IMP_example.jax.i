// Add JAX implementations to each of our example classes.
// [JAX](https://docs.jax.dev/) ia high performance Python library
// which has NumPy-like syntax but compiles to fast code that will
// run on both CPUs and GPUs.

// In IMP, we can run the entire scoring and sampling procedure on
// a GPU only if every part of the scoring function (e.g. restraints,
// constraints) has a JAX implementation. Generally this is done by
// providing a _get_jax() method in the Python class that returns
// one or more JAX functions.

%extend IMP::example::ExampleRestraint {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the restraint using JAX.
           For a Restraint, we must return a single JAX function which
           takes a single argument, the JAX Model, and returns its score.
           The JAX Model (here called `jm`) is a JAX object which contains
           the same information as the IMP Model, as a simple Python dict.
           For example, jm['xyz'] is an N x 3 array of XYZ coordinates."""
        import functools
        def jax_restraint(jm, k, pi):
            # Get the xyz coordinates for particle pi in the JAX Model
            xyz = jm['xyz'][pi]
            # Score the Z coordinate. Note that we do not need to calculate
            # its first derivatives; this is handled automatically by JAX.
            return 0.5 * k * xyz[2] * xyz[2]
        # We must return a function which takes only one argument, `jm`.
        # Here we use functools.partial to "bake in" the other parameters
        # k and pi, getting their values from the IMP Restraint object.
        f = functools.partial(jax_restraint, k=self.get_force_constant(),
                              pi=self.get_index())
        # Finally, we use the helper method _wrap_jax to return an object
        # that contains our JAX function and other restraint-specific
        # information, such as the weight
        return self._wrap_jax(f)
  %}
}

%extend IMP::example::ExampleConstraint {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the constraint using JAX.
           Similar to a Restraint, we must return a single JAX function which
           takes as input a JAX Model. However, in this case the function
           must return a new JAX Model, with the constraint applied."""
        import functools
        def apply_func(jm, key, index):
            # JAX arrays are immutable so we cannot simply say
            # jm[key][index] += 1
            # Instead, we must return a new array which is a modified
            # version of the original using the JAX `at` method
            jm[key] = jm[key].at[index].add(1)
            return jm
        key = self.get_key()
        f = functools.partial(apply_func, key=key.get_string(),
                              index=self.get_index())
        # _wrap_jax also takes an optional `keys` argument which is a list
        # of the IMP attribute keys used by the JAX function. These will be
        # added to the JAX Model object (although the Model will always
        # have `xyz` and `r` keys, for coordinates and radii).
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

%extend IMP::example::ExampleUnaryFunction {
  %pythoncode %{
    def _get_jax(self):
        """Implementation of the unary function using JAX"""
        import functools
        def score(feature, center, k):
            return 0.5 * k * (feature - center) ** 2
        return functools.partial(score, center=self.get_center(),
                                 k=self.get_force_constant())
  %}
}
