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

%extend IMP::core::WeightedSum {
  %pythoncode %{
    def _get_jax(self):
        import functools
        import jax.numpy as jnp
        def score(val, funcs, weights):
            return sum(f(val) * weight for (f, weight) in zip(funcs, weights))
        nfunc = self.get_function_number()
        funcs = [self.get_function(i).get_derived_object()._get_jax()
                 for i in range(nfunc)]
        return functools.partial(score, funcs=funcs,
                                 weights=self.get_weights())
  %}
}

%extend IMP::core::WeightedSumOfExponential {
  %pythoncode %{
    def _get_jax(self):
        import functools
        import jax.numpy as jnp
        def score(val, funcs, weights, denom):
            exp_sum = sum(weight * jnp.exp(-f(val) / denom)
                          for (f, weight) in zip(funcs, weights))
            return -jnp.log(exp_sum) * denom
        nfunc = self.get_function_number()
        funcs = [self.get_function(i).get_derived_object()._get_jax()
                 for i in range(nfunc)]
        return functools.partial(score, funcs=funcs,
                                 weights=self.get_weights(),
                                 denom=self.get_denominator())
  %}
}

%extend IMP::core::GenericDistanceToSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def score(jm, indexes, point, uf):
            xyzs = jm['xyz'][indexes]
            drs = jnp.linalg.norm(xyzs - point, axis=1)
            return uf(drs)
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, point=jnp.array(self.get_point()),
                              uf=uf._get_jax())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::GenericBoundingBox3DSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def score(jm, indexes, box_min, box_max, uf):
            xyzs = jm['xyz'][indexes]
            # This calculates the distance and the score for every point,
            # even those inside the box. IMP just returns zero for points
            # inside the box, skipping the distance calculation.
            # The implementation here is *probably* faster on a GPU since
            # we reduce the use of conditionals, and JAX will likely skip
            # the sqrt if uf is a harmonic, but this should be benchmarked.
            drs = jnp.linalg.norm(
                xyzs - jnp.clip(xyzs, box_min, box_max), axis=1)
            return uf(drs)
        uf = self.get_unary_function().get_derived_object()
        bb = self.get_bounding_box()
        f = functools.partial(score, box_min=jnp.asarray(bb.get_corner(0)),
                              box_max=jnp.asarray(bb.get_corner(1)),
                              uf=uf._get_jax())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::HarmonicDistancePairScore {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def jax_harmonic_distance_pair_score(jm, indexes, d, k):
            xyzs = jm['xyz'][indexes]
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
        def jax_score(jm, indexes, d, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::DistancePairScore {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        import functools
        def jax_score(jm, indexes, uf):
            xyzs = jm['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            return uf(drs)
        sfnc = self.get_score_functor()
        uf = sfnc.get_unary_function().get_derived_object()
        f = functools.partial(jax_score, uf=uf._get_jax())
        return self._wrap_jax(f)
  %}
}

%extend IMP::core::DistanceRestraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        ps = self.get_score_object()
        ji = ps._get_jax()
        indexes = jnp.array([self.get_index()])
        return self._wrap_jax(lambda jm: ji.score_func(jm, indexes))
  %}
}

%extend IMP::core::RestraintsScoringFunction {
  %pythoncode %{
    def _get_jax(self):
        import IMP._jax_util
        jis = [r.get_derived_object()._get_jax() for r in self.restraints]
        funcs = [j.score_func for j in jis]
        keys = frozenset(x for j in jis for x in j._keys)
        def jax_sf(jm):
            return sum(f(jm) for f in funcs)
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

%extend IMP::core::SingletonConstraint {
  %pythoncode %{
    def _get_jax(self):
        import functools
        index = self.get_index()
        mod = self.get_before_modifier().get_derived_object()
        ji = mod._get_jax(self.get_model(), index)
        return self._wrap_jax(
            functools.partial(ji.apply_func, indexes=index),
            keys=ji._keys)
  %}
}

%extend IMP::core::CentroidOfRefined {
  %pythoncode %{
    def _get_jax(self, m, index=None):
        import functools
        import jax.numpy as jnp
        if index is None:
            raise NotImplementedError("Only implemented for single particle")
        refined = self.get_refiner().get_refined_indexes(m, index)

        def apply_func_unweighted(jm, indexes):
            xyz = jm['xyz']
            jm['xyz'] = xyz.at[indexes].set(jnp.average(xyz[refined], axis=0))
            return jm

        def apply_func_weighted(jm, indexes, weight_key):
            xyz = jm['xyz']
            weights = jm[weight_key][refined]
            jm['xyz'] = xyz.at[indexes].set(jnp.average(xyz[refined], axis=0,
                                                        weights=weights))
            return jm

        keys = frozenset(self.get_keys())
        if keys != frozenset(IMP.core.XYZ.get_xyz_keys()):
            raise NotImplementedError("Only works for XYZ")

        if self.get_is_weight_null():
            return self._wrap_jax(apply_func_unweighted)
        else:
            weight_key = self.get_weight()
            return self._wrap_jax(
                functools.partial(apply_func_weighted,
                                  weight_key=weight_key.get_string()),
                keys=(weight_key,))
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
                  JAX Model and the mover's persistent state object.
                  It should return a new JAX Model with the move applied,
                  a new persistent state, and the proposal ratio. If the move
                  is rejected then the new JAX Model will be discarded.
                  However, the mover's persistent state is updated for both
                  accepted and rejected moves.
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

        def propose_func(jm, key):
            key, subkey = jax.random.split(key)
            v = get_random_vector_in_3d_sphere(subkey, radius)
            jm['xyz'] = jm['xyz'].at[indexes].add(v)
            return jm, key, 1.0
        return self._wrap_jax(init_func, propose_func)
  %}
}

%extend IMP::core::SerialMover {
  %pythoncode %{
    def _get_jax(self):
        import jax.random
        import jax.lax
        import functools
        from IMP.core._jax_util import _SerialMover
        movers = [m.get_derived_object()._get_jax()
                  for m in self.get_movers()]

        def sub_propose_func(jm, sms, i):
            """Call the propose_func of the ith mover"""
            jm, sms.mover_state[i], ratio = movers[i].propose_func(
                jm, sms.mover_state[i])
            return jm, sms, ratio

        sub_propose_funcs = [functools.partial(sub_propose_func, i=i)
                             for i in range(len(movers))]

        def init_func(key):
            # Initialize all movers and store their state in ours
            mover_state = []
            for m in movers:
                key, subkey = jax.random.split(key)
                mover_state.append(m.init_func(subkey))
            return _SerialMover(imov=-1, mover_state=mover_state)

        def propose_func(jm, sms):
            sms.imov = jax.lax.min(sms.imov + 1, len(movers) - 1)
            return jax.lax.switch(sms.imov, sub_propose_funcs, jm, sms)
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
