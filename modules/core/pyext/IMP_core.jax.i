%pythonbegin %{
  import functools
%}

%extend IMP::core::Harmonic {
  %pythoncode %{
    def _get_jax(self):
        def score(val, mean, k):
            return 0.5 * k * (mean - val) ** 2
        return functools.partial(score, mean=self.get_mean(), k=self.get_k())
  %}
}

%extend IMP::core::HarmonicUpperBound {
  %pythoncode %{
    def _get_jax(self):
        import jax.lax
        def score(val, mean, k):
            return 0.5 * k * jax.lax.min(mean - val, 0.0) ** 2
        return functools.partial(score, mean=self.get_mean(), k=self.get_k())
  %}
}

%extend IMP::core::HarmonicLowerBound {
  %pythoncode %{
    def _get_jax(self):
        import jax.lax
        def score(val, mean, k):
            return 0.5 * k * jax.lax.max(mean - val, 0.0) ** 2
        return functools.partial(score, mean=self.get_mean(), k=self.get_k())
  %}
}

%extend IMP::core::HarmonicWell {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        def score(val, lb, ub, k):
            return 0.5 * k * (val - jnp.clip(val, lb, ub)) ** 2
        well = self.get_well()
        return functools.partial(score, lb=well[0], ub=well[1], k=self.get_k())
  %}
}

%extend IMP::core::Linear {
  %pythoncode %{
    def _get_jax(self):
        def score(val, slope, offset):
            return (val - offset) * slope
        return functools.partial(score, slope=self.get_slope(),
                                 offset=self.get_offset())
  %}
}

%extend IMP::core::Cosine {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        def score(val, k, period, phase):
            return jnp.abs(k) - k * jnp.cos(period * val + phase)
        return functools.partial(score, k=self.get_force_constant(),
                                 period=self.get_periodicity(),
                                 phase=self.get_phase())
  %}
}

%extend IMP::core::WeightedSum {
  %pythoncode %{
    def _get_jax(self):
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

%extend IMP::core::ClosedCubicSpline {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        from IMP.core._jax_util import _spline
        def score(feature, minrange, spacing, values, second_derivs):
            # determine bin index and thus the cubic fragment to use:
            lowbin = jnp.array((feature - minrange) / spacing, dtype=int)
            # enforce periodicity - wrap around from n to 0
            highbin = jnp.remainder(lowbin + 1, len(values))
            return _spline(feature, minrange, lowbin, highbin, spacing,
                           values, second_derivs)
        return functools.partial(
            score, minrange=self.get_minrange(),
            spacing=self.get_spacing(), values=jnp.asarray(self.get_values()),
            second_derivs=jnp.asarray(self.get_second_derivatives()))
  %}
}

%extend IMP::core::OpenCubicSpline {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        from IMP.core._jax_util import _spline
        def score(feature, minrange, maxrange, spacing, values, second_derivs):
            # clip feature to range (extend=True behavior)
            feature = jnp.clip(feature, minrange, maxrange)
            # determine bin index and thus the cubic fragment to use:
            lowbin = jnp.array((feature - minrange) / spacing, dtype=int)
            return _spline(feature, minrange, lowbin, lowbin + 1, spacing,
                           values, second_derivs)
        spacing = self.get_spacing()
        minrange = self.get_minrange()
        values = jnp.asarray(self.get_values())
        maxrange = minrange + spacing * (len(values) - 1)
        return functools.partial(
            score, minrange=minrange, maxrange=maxrange,
            spacing=spacing, values=values,
            second_derivs=jnp.asarray(self.get_second_derivatives()))
  %}
}

%extend IMP::core::GenericDistanceToSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        def score(jm, point, uf):
            xyzs = jm['xyz'][indexes]
            drs = space.distance(xyzs - point)
            return uf(drs)
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, point=jnp.array(self.get_point()),
                              uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::GenericBoundingBox3DSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        def score(jm, box_min, box_max, uf):
            xyzs = jm['xyz'][indexes]
            diff = xyzs - jnp.clip(xyzs, box_min, box_max)
            outside = jnp.any(diff != 0., axis=-1)
            # Avoid taking the distance of a zero vector for points inside
            # the box, since the gradient of sqrt(0) is infinite (NaN)
            safe_diff = jnp.where(outside[..., jnp.newaxis], diff, 1.)
            drs = jnp.where(outside, space.distance(safe_diff), 0.)
            return jnp.where(outside, uf(drs), 0.)
        uf = self.get_unary_function().get_derived_object()
        bb = self.get_bounding_box()
        f = functools.partial(score, box_min=jnp.asarray(bb.get_corner(0)),
                              box_max=jnp.asarray(bb.get_corner(1)),
                              uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::GenericBoundingSphere3DSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import numpy as np
        import jax.numpy as jnp
        def score(jm, use_radii, center, radius, uf):
            xyzs = jm['xyz'][indexes]
            radii = jnp.where(use_radii, jm['r'][indexes], 0.0)
            drs = space.distance(xyzs - center) + radii - radius
            return jnp.where(drs < 0.000001, 0.0, uf(drs))
        use_radii = np.array([XYZR.get_is_setup(m, IMP.ParticleIndex(ind))
                              for ind in indexes])
        uf = self.get_unary_function().get_derived_object()
        sphere = self.get_sphere()
        f = functools.partial(score, use_radii=use_radii,
                              center=jnp.asarray(sphere.get_center()),
                              radius=sphere.get_radius(), uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::GenericAttributeSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        def score_float_key(jm, key, uf):
            return uf(jm[key][indexes])

        def score_xyz_key(jm, xyz_index, uf):
            return uf(jm['xyz'][indexes, xyz_index])

        uf = self.get_unary_function().get_derived_object()._get_jax()
        key = self.get_key()
        # First 7 FloatKeys are reserved in IMP and have to be handled
        # specially
        if key.get_index() >= 7:
            need_keys = [key]
            f = functools.partial(score_float_key, key=key.get_string(), uf=uf)
        elif key == XYZR.get_radius_key():
            need_keys = []  # We already have the radius in the JAX model
            f = functools.partial(score_float_key, key='r', uf=uf)
        elif key in XYZ.get_xyz_keys():
            need_keys = []  # We already have coordinates in the JAX model
            xyz_index = XYZ.get_xyz_keys().index(key)
            f = functools.partial(score_xyz_key, xyz_index=xyz_index, uf=uf)
        else:
            raise NotImplementedError("No support for key %s" % key)
        return self._wrap_jax(m, f, keys=need_keys)
  %}
}

%extend IMP::core::HarmonicDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        def jax_harmonic_distance_pair_score(jm, d, k):
            xyzs = jm['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = space.distance(diff)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_harmonic_distance_pair_score,
                              d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::HarmonicSphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        def jax_score(jm, d, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = space.distance(diff) - rs.sum(axis=-1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::HarmonicUpperBoundSphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        import jax.lax
        def jax_score(jm, d, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = space.distance(diff) - rs.sum(axis=-1)
            return 0.5 * k * jax.lax.min(d - drs, 0.0) ** 2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::SoftSpherePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        import jax.lax
        def jax_score(jm, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = space.distance(diff) - rs.sum(axis=-1)
            return 0.5 * k * jax.lax.min(drs, 0.0) ** 2
        f = functools.partial(jax_score, k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::SphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        def jax_score(jm, uf):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = space.distance(diff) - rs.sum(axis=-1)
            return uf(drs)
        sfnc = self.get_score_functor()
        uf = sfnc.get_unary_function().get_derived_object()
        f = functools.partial(jax_score, uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::DistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes, space):
        import jax.numpy as jnp
        def jax_score(jm, uf):
            xyzs = jm['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = space.distance(diff)
            return uf(drs)
        sfnc = self.get_score_functor()
        uf = sfnc.get_unary_function().get_derived_object()
        f = functools.partial(jax_score, uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::DistanceRestraint {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.numpy as jnp
        import IMP._jax_util
        ps = self.get_score_object()
        indexes = jnp.array([self.get_index()])
        ji = ps._get_jax(self.get_model(), indexes, space=space)
        def score(jm):
            return jnp.sum(ji.score_func(jm), axis=-1)
        return self._wrap_jax(score)
  %}
}

%extend IMP::core::ExcludedVolumeRestraint {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.numpy as jnp
        # We need all indexes for JAX but ExcludedVolumeRestraint does not
        # provide a suitable method, so make a temporary ClosePairContainer
        # instead with the same set of indexes (we use all pairs for JAX so
        # the cutoff and slack are unimportant here)
        c = IMP.container.ClosePairContainer(self.get_container(), 10.0, 1.0)
        c.pair_filters.extend(self.pair_filters)
        indexes = c._get_static_contents()

        score = self.get_score_object().get_derived_object()
        ji = score._get_jax(self.get_model(), indexes, space)
        score_jax = ji.score_func

        def jax_restraint(jm):
            return jnp.sum(score_jax(jm), axis=-1)
        return self._wrap_jax(jax_restraint, keys=ji._keys)
  %}
}

%extend IMP::core::RestraintsScoringFunction {
  %pythoncode %{
    def _get_jax(self, space=None):
        import jax.numpy as jnp
        import IMP.jax
        import IMP._jax_util
        restraints = [r.get_derived_object() for r in self.restraints]
        space = space or IMP.jax.FreeSpace
        multi_funcs, funcs, keys = IMP._jax_util.get_restraints_jax_funcs_keys(
            restraints, space)
        def jax_sf(jm):
            rets = [f(jm) for f in multi_funcs]
            if funcs:
                rets.append(jnp.stack([f(jm) for f in funcs], axis=-1))
            scores = jnp.concatenate(rets, axis=-1)
            return jnp.sum(scores, axis=-1)
        return IMP._jax_util.JAXRestraintInfo(
            m=self.get_model(), score_func=jax_sf, weight=1.0, keys=keys)
  %}
}

%extend IMP::core::SingletonRestraint {
  %pythoncode %{
    def _get_jax(self, space):
        from . import _jax_util
        return _jax_util._get_jax_restraint(self, space)

    @classmethod
    def _get_jax_multiple(cls, restraints, space):
        from . import _jax_util
        return _jax_util._get_jax_multi_restraint(cls, restraints, space)
  %}
}

%extend IMP::core::PairRestraint {
  %pythoncode %{
    def _get_jax(self, space):
        from . import _jax_util
        return _jax_util._get_jax_restraint(self, space)

    @classmethod
    def _get_jax_multiple(cls, restraints, space):
        from . import _jax_util
        return _jax_util._get_jax_multi_restraint(cls, restraints, space)
  %}
}

%extend IMP::core::SingletonConstraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        indexes = jnp.array([self.get_index()])
        mod = self.get_before_modifier().get_derived_object()
        ji = mod._get_jax(self.get_model(), indexes)
        return self._wrap_jax(ji.apply_func, keys=ji._keys)
  %}
}

%extend IMP::core::CentroidOfRefined {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        refined = [
            self.get_refiner().get_refined_indexes(m, IMP.ParticleIndex(index))
            for index in indexes]

        def apply_func_unweighted(jm):
            for ind, ref in zip(indexes, refined):
                xyz = jm['xyz']
                jm['xyz'] = xyz.at[ind].set(jnp.average(xyz[ref], axis=0))
            return jm

        def apply_func_weighted(jm, weight_key):
            for ind, ref in zip(indexes, refined):
                xyz = jm['xyz']
                weights = jm[weight_key][ref]
                jm['xyz'] = xyz.at[ind].set(
                    jnp.average(xyz[ref], axis=0, weights=weights))
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
    def _get_jax(self, space):
        """Return a JAX implementation of this mover.
           Implement this method in a MonteCarloMover subclass to provide
           equivalent functionality using [JAX](https://docs.jax.dev/).
           See _wrap_jax for more information.

           @param space The space in which the mover operates
                  (e.g. a periodic box). See IMP._jax_util.Space.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")

    def _wrap_jax(self, init_func, propose_func, accept_func=None,
                  sync_func=None, reset_stats_func=None, keys=None):
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
           @param accept_func if provided, a JAX function which is called
                  after each accepted Monte Carlo move, with the persistent
                  state object. It should return a new persistent state.
           @param sync_func If provided, a Python function which is called
                  at the end of a Monte Carlo sampling run to sync mover
                  data back to IMP. It is called with the persistent state
                  and the IMP Mover object.
           @param reset_stats_func If provided, a JAX function which is called
                  at the beginning of a Monte Carlo run to reset this mover's
                  statistics. It is called with the persistent state object and
                  should return a new state.
           @param keys If given, a set of IMP::Key objects describing Model
                  attributes (other than xyz and radius) that are altered
                  by this mover.
        """
        from IMP.core._jax_util import JAXMoverInfo
        return JAXMoverInfo(init_func, propose_func, accept_func, sync_func,
                            reset_stats_func, keys)
  %}
}

%extend IMP::core::BallMover {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.random
        from IMP.algebra.jax import get_random_vector_in_3d_sphere
        indexes = self.get_indexes()
        keys = frozenset(self.get_keys())
        if keys == frozenset(IMP.core.XYZ.get_xyz_keys()):
            intcoord = False
        elif keys == frozenset(
                IMP.core.RigidBodyMember.get_internal_coordinate_keys()):
            intcoord = True
        else:
            raise NotImplementedError(
                "Only works for XYZ or internal coordinates")
        radius = self.get_radius()

        def init_func(key):
            return key

        def propose_func(jm, key):
            key, subkey = jax.random.split(key)
            if intcoord:
                rbs = jm['rigid_bodies']
                val = rbs.intcoord
            else:
                val = jm['xyz']
            v = get_random_vector_in_3d_sphere(
                subkey, radius, shape=val.shape[:-2] + (1,))
            if intcoord:
                rbs.intcoord = space.shift_indexes(val, indexes, v)
            else:
                jm['xyz'] = space.shift_indexes(val, indexes, v)
            return jm, key, 1.0
        return self._wrap_jax(init_func, propose_func,
                              keys=['rigid_bodies'] if intcoord else None)
  %}
}

%extend IMP::core::RigidBodyMover {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.random
        import IMP.core._jax_rigid
        from IMP.algebra.jax import (get_random_vector_on_3d_sphere,
                                     get_random_vector_in_3d_sphere,
                                     get_rotation_about_normalized_axis,
                                     _quaternion_multiply)
        max_angle = self.get_maximum_rotation()
        max_translation = self.get_maximum_translation()
        particle_index = self.get_index()
        body_index = IMP.core._jax_rigid._get_rigid_body_index(
            self.get_model(), self.get_index())

        def init_func(key):
            return key

        def propose_func(jm, key):
            body = jm['rigid_bodies'].bodies[body_index]
            tf = body.get_transformation(jm)

            if max_translation > 0:
                key, subkey = jax.random.split(key)
                tf.translation = space.shift(
                    tf.translation,
                    get_random_vector_in_3d_sphere(subkey, max_translation)[0])
            if max_angle > 0:
                key, subkey1, subkey2 = jax.random.split(key, 3)
                axis_norm = get_random_vector_on_3d_sphere(subkey1, 1.0)[0]
                angle = jax.random.uniform(
                    subkey2, minval=-max_angle, maxval=max_angle)
                rotation = get_rotation_about_normalized_axis(axis_norm, angle)
                tf.rotation = _quaternion_multiply(rotation, tf.rotation)
            jm = body.set_transformation_lazy(tf, jm)
            return jm, key, 1.0

        return self._wrap_jax(init_func, propose_func, keys=['rigid_bodies'])
  %}
}

%extend IMP::core::SerialMover {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.random
        import jax.lax
        import jax.numpy as jnp
        from IMP.core._jax_util import _SerialMover
        movers = [m.get_derived_object()._get_jax(space)
                  for m in self.get_movers()]

        def sub_propose_func(jm, sms, i):
            """Call the propose_func of the ith mover"""
            jm, sms.mover_state[i], ratio = movers[i].propose_func(
                jm, sms.mover_state[i])
            sms.proposed_mover_steps = sms.proposed_mover_steps.at[i].add(1)
            return jm, sms, ratio

        sub_propose_funcs = [functools.partial(sub_propose_func, i=i)
                             for i in range(len(movers))]

        def init_func(key):
            # Initialize all movers and store their state in ours
            mover_state = []
            for m in movers:
                key, subkey = jax.random.split(key)
                mover_state.append(m.init_func(subkey))
            return _SerialMover(
                imov=-1, mover_state=mover_state,
                proposed_mover_steps=jnp.zeros(len(movers), dtype=int),
                accepted_mover_steps=jnp.zeros(len(movers), dtype=int))

        def reset_stats_func(sms):
            sms.mover_state = [m.reset_stats_func(ms)
                               if m.reset_stats_func else ms
                               for (m, ms) in zip(movers, sms.mover_state)]
            sms.proposed_mover_steps = jnp.zeros(len(movers), dtype=int)
            sms.accepted_mover_steps = jnp.zeros(len(movers), dtype=int)
            return sms

        def propose_func(jm, sms):
            sms.imov = jnp.mod(sms.imov + 1, len(movers))
            return jax.lax.switch(sms.imov, sub_propose_funcs, jm, sms)

        def accept_func(sms):
            # Update statistics for the chosen mover
            sms.accepted_mover_steps = \
                sms.accepted_mover_steps.at[sms.imov].add(1)
            return sms

        def sync_func(imp_mover, sms):
            # Copy submover statistics back to IMP Movers
            for i, mover in enumerate(imp_mover.get_movers()):
                mover.add_to_statistics(
                    sms.proposed_mover_steps[i],
                    sms.proposed_mover_steps[i] - sms.accepted_mover_steps[i])

        return self._wrap_jax(
            init_func, propose_func, accept_func, sync_func,
            reset_stats_func=reset_stats_func,
            keys=frozenset(x for m in movers for x in m._keys))
  %}
}

%extend IMP::core::MonteCarlo {
  %pythoncode %{
    def _get_jax(self, space=None):
        from IMP.core._jax_util import _MCJAXInfo
        return _MCJAXInfo(self, space)

    def _get_jax_optimizer(self, max_steps, space=None):
        import IMP.core._jax_util
        return IMP.core._jax_util._MCJAXOptimizer(self, max_steps, space)

    def _optimize_jax(self, max_steps, space=None):
        opt = self._get_jax_optimizer(max_steps, space=space)
        score, mc_state = opt.optimize(opt.get_initial_state())
        return score
  %}
}

%extend IMP::core::internal::_RigidBodyNormalizeConstraint {
  %pythoncode %{
    def _get_jax(self):
        from IMP.algebra.jax import _quaternion_normalize
        import jax
        def apply_func(jm):
            # Assume that constraint acts on all rigid bodies
            allrbs = jm['rigid_bodies']
            allrbs.quaternion = _quaternion_normalize(allrbs.quaternion)
            return jm
        return self._wrap_jax(apply_func, keys=['rigid_bodies'])
  %}
}

%extend IMP::core::internal::_RigidBodyPositionConstraint {
  %pythoncode %{
    def _get_jax(self):
        import jax
        from IMP.core import _jax_rigid
        def apply_func(jm, index):
            body = jm['rigid_bodies'].bodies[index]
            # todo: this could perhaps be better parallelized, as in most
            # cases (at least, without nested rigid bodies) the update of
            # one rigid body does not affect members of another body
            return body.update_members(jm)
        # Get rigid body index from particle index
        pindex = self.get_index()
        rbindex = _jax_rigid._get_rigid_body_index(self.get_model(), pindex)
        f = functools.partial(apply_func, index=rbindex)
        return self._wrap_jax(f, keys=['rigid_bodies'])
  %}
}

%extend IMP::core::NormalMover {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.random
        from IMP.core._jax_util import _get_offset_propose_func
        keys = self.get_keys()
        sigma = self.get_sigma()

        def offset_func(key, shape):
            key, subkey = jax.random.split(key)
            return key, jax.random.normal(subkey, shape) * sigma

        propose_func, keys = _get_offset_propose_func(
            self.get_indexes(), keys, offset_func)

        return self._wrap_jax(lambda key: key, propose_func, keys=keys)
  %}
}

%extend IMP::core::LogNormalMover {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.random
        from IMP.core._jax_util import _get_offset_propose_func
        keys = self.get_keys()
        sigma = self.get_sigma()

        def offset_func(key, shape):
            key, subkey = jax.random.split(key)
            return key, jax.random.lognormal(subkey, sigma, shape)

        propose_func, keys = _get_offset_propose_func(
            self.get_indexes(), keys, offset_func)

        return self._wrap_jax(lambda key: key, propose_func, keys=keys)
  %}
}
