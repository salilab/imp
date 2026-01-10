%extend IMP::core::Harmonic {
  %pythoncode %{
    def _get_jax(self):
        import functools
        def score(val, mean, k):
            return 0.5 * k * (mean - val) ** 2
        return functools.partial(score, mean=self.get_mean(), k=self.get_k())
  %}
}

%extend IMP::core::HarmonicUpperBound {
  %pythoncode %{
    def _get_jax(self):
        import functools
        import jax.lax
        def score(val, mean, k):
            return 0.5 * k * jax.lax.min(mean - val, 0.0) ** 2
        return functools.partial(score, mean=self.get_mean(), k=self.get_k())
  %}
}

%extend IMP::core::HarmonicLowerBound {
  %pythoncode %{
    def _get_jax(self):
        import functools
        import jax.lax
        def score(val, mean, k):
            return 0.5 * k * jax.lax.max(mean - val, 0.0) ** 2
        return functools.partial(score, mean=self.get_mean(), k=self.get_k())
  %}
}

%extend IMP::core::Linear {
  %pythoncode %{
    def _get_jax(self):
        import functools
        def score(val, slope, offset):
            return (val - offset) * slope
        return functools.partial(score, slope=self.get_slope(),
                                 offset=self.get_offset())
  %}
}

%extend IMP::core::GenericDistanceToSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def score(X, indexes, point, uf):
            xyzs = X['xyz'][indexes]
            drs = jnp.linalg.norm(xyzs - point, axis=1)
            return uf(drs)
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, point=jnp.array(self.get_point()),
                              uf=uf._get_jax())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::HarmonicDistancePairScore {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def jax_harmonic_distance_pair_score(X, indexes, d, k):
            xyzs = X['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_harmonic_distance_pair_score,
                              d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::HarmonicSphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def jax_score(X, indexes, d, k):
            xyzs = X['xyz'][indexes]
            rs = X['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::RestraintsScoringFunction {
  %pythoncode %{
    def _get_jax(self):
        import IMP._jax_util
        jis = [r.get_derived_object()._get_jax() for r in self.restraints]
        funcs = [j.score_func for j in jis]
        keys = frozenset(x for j in jis for x in j._keys)
        def jax_sf(X):
            return sum(f(X) for f in funcs)
        return IMP._jax_util.JAXRestraintInfo(
            m=self.get_model(), score_func=jax_sf, weight=1.0, keys=keys)
  %}
}

%extend IMP::core::SingletonRestraint {
  %pythoncode %{
    def _get_jax(self):
        from . import _jax_util
        return _jax_util._get_jax_restraint(self)
  %}
}

%extend IMP::core::PairRestraint {
  %pythoncode %{
    def _get_jax(self):
        from . import _jax_util
        return _jax_util._get_jax_restraint(self)
  %}
}

%extend IMP::core::MonteCarloMover {
  %pythoncode %{
    def _get_jax(self):
        """Return a JAX implementation of this mover.
           Implement this method in a MonteCarloMover subclass to provide
           equivalent functionality using [JAX](https://docs.jax.dev/).
           See _wrap_jax for more information.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")

    def _wrap_jax(self, init_func, propose_func):
        """Create the return value for _get_jax.
           Use this method in _get_jax() to wrap the JAX functions
           with other mover-specific information.

           @param init_func a JAX function which is used to initialize this
                  mover. It is called with a single argument, a fresh JAX
                  random key, and should return a persistent state object.
                  This object may be the key itself, or any other Python object
                  that JAX understands.
           @param propose_func a JAX function which is called with the current
                  model state and the mover's persistent state object.
                  It should return a new model state, a new persistent state,
                  and the proposal ratio.
        """
        from IMP.core._jax_util import JAXMoverInfo
        return JAXMoverInfo(init_func, propose_func)
  %}
}

%extend IMP::core::BallMover {
  %pythoncode %{
    def _get_jax(self):
        import jax.random
        from IMP.algebra._jax_util import get_random_vector_in_3d_sphere
        indexes = self.get_indexes()
        keys = frozenset(self.get_keys())
        if keys != frozenset(IMP.core.XYZ.get_xyz_keys()):
            raise NotImplementedError("Only works for XYZ")
        radius = self.get_radius()

        def init_func(key):
            return key

        def propose_func(X, key):
            key, subkey = jax.random.split(key)
            v = get_random_vector_in_3d_sphere(subkey, radius)
            newX = X.copy()
            newX['xyz'] = X['xyz'].at[indexes].add(v)
            return newX, key, 1.0
        return self._wrap_jax(init_func, propose_func)
  %}
}

%extend IMP::core::SerialMover {
  %pythoncode %{
    def _get_jax(self):
        import jax.random
        import jax.lax
        import functools
        from IMP.core._jax_util import _SerialMoverState
        movers = [m.get_derived_object()._get_jax()
                  for m in self.get_movers()]

        def sub_propose_func(X, sms, i):
            """Call the propose_func of the ith mover"""
            X, sms.mover_state[i], ratio = movers[i].propose_func(
                X, sms.mover_state[i])
            return X, sms, ratio

        sub_propose_funcs = [functools.partial(sub_propose_func, i=i)
                             for i in range(len(movers))]

        def init_func(key):
            # Initialize all movers and store their state in ours
            mover_state = []
            for m in movers:
                key, subkey = jax.random.split(key)
                mover_state.append(m.init_func(subkey))
            return _SerialMoverState(imov=-1, mover_state=mover_state)

        def propose_func(X, sms):
            sms.imov = jax.lax.min(sms.imov + 1, len(movers) - 1)
            return jax.lax.switch(sms.imov, sub_propose_funcs, X, sms)
        return self._wrap_jax(init_func, propose_func)
  %}
}

%extend IMP::core::MonteCarlo {
  %pythoncode %{
    def _get_jax(self):
        from IMP.core._jax_util import _MCJAXInfo
        return _MCJAXInfo(self)

    def _optimize_jax(self, max_steps):
        import IMP.core._jax_util
        return IMP.core._jax_util._mc_optimize(self, max_steps)
  %}
}
