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
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        def score(jm, point, uf):
            xyzs = jm['xyz'][indexes]
            drs = jnp.linalg.norm(xyzs - point, axis=1)
            return uf(drs)
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, point=jnp.array(self.get_point()),
                              uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::GenericBoundingBox3DSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        def score(jm, box_min, box_max, uf):
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
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::GenericBoundingSphere3DSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        import jax.lax
        def score_with_radius(jm, inds, center, radius):
            xyzs = jm['xyz'][inds]
            radii = jm['r'][inds]
            drs = jnp.linalg.norm(xyzs - center) + radii - radius
            return jax.lax.select(drs < 0.000001, jnp.zeros_like(drs), uf(drs))
        def score_without_radius(jm, inds, center, radius):
            xyzs = jm['xyz'][inds]
            drs = jnp.linalg.norm(xyzs - center) - radius
            return jax.lax.select(drs < 0.000001, jnp.zeros_like(drs), uf(drs))
        without_radii_inds = []
        with_radii_inds = []
        for ind in indexes:
            if XYZR.get_is_setup(m, ind):
                with_radii_inds.append(ind)
            else:
                without_radii_inds.append(ind)
        without_radii_inds = jnp.asarray(without_radii_inds)
        with_radii_inds = jnp.asarray(with_radii_inds)
        uf = self.get_unary_function().get_derived_object()._get_jax()
        sphere = self.get_sphere()
        radius = sphere.get_radius()
        center = jnp.asarray(sphere.get_center())
        def score(jm):
            s = 0.
            if without_radii_inds.size > 0:
                s += score_without_radius(jm, without_radii_inds,
                                          center, radius)
            if with_radii_inds.size > 0:
                s += score_with_radius(jm, with_radii_inds, center, radius)
            return s
        return self._wrap_jax(m, score)
  %}
}

%extend IMP::core::GenericAttributeSingletonScore<UnaryFunction> {
  %pythoncode %{
    def _get_jax(self, m, indexes):
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
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        def jax_harmonic_distance_pair_score(jm, d, k):
            xyzs = jm['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_harmonic_distance_pair_score,
                              d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::HarmonicSphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        def jax_score(jm, d, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return 0.5 * k * (d - drs)**2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::HarmonicUpperBoundSphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        import jax.lax
        def jax_score(jm, d, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return 0.5 * k * jax.lax.min(d - drs, 0.0) ** 2
        f = functools.partial(jax_score, d=self.get_x0(), k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::SoftSpherePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        import jax.lax
        def jax_score(jm, k):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return 0.5 * k * jax.lax.min(drs, 0.0) ** 2
        f = functools.partial(jax_score, k=self.get_k())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::SphereDistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        def jax_score(jm, uf):
            xyzs = jm['xyz'][indexes]
            rs = jm['r'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1) - rs.sum(axis=1)
            return uf(drs)
        sfnc = self.get_score_functor()
        uf = sfnc.get_unary_function().get_derived_object()
        f = functools.partial(jax_score, uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::DistancePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        def jax_score(jm, uf):
            xyzs = jm['xyz'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            return uf(drs)
        sfnc = self.get_score_functor()
        uf = sfnc.get_unary_function().get_derived_object()
        f = functools.partial(jax_score, uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::core::DistanceRestraint {
  %pythoncode %{
    def _get_jax(self):
        import jax.numpy as jnp
        ps = self.get_score_object()
        indexes = jnp.array([self.get_index()])
        ji = ps._get_jax(self.get_model(), indexes)
        def score(jm):
            return jnp.sum(ji.score_func(jm))
        return self._wrap_jax(score)
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
    def _get_jax(self):
        """Return a JAX implementation of this mover.
           Implement this method in a MonteCarloMover subclass to provide
           equivalent functionality using [JAX](https://docs.jax.dev/).
           See _wrap_jax for more information.
        """
        raise NotImplementedError(f"No JAX implementation for {self}")

    def _wrap_jax(self, init_func, propose_func, accept_func=None,
                  sync_func=None, keys=None):
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
           @param keys If given, a set of IMP::Key objects describing Model
                  attributes (other than xyz and radius) that are altered
                  by this mover.
        """
        from IMP.core._jax_util import JAXMoverInfo
        return JAXMoverInfo(init_func, propose_func, accept_func, sync_func,
                            keys)
  %}
}

%extend IMP::core::BallMover {
  %pythoncode %{
    def _get_jax(self):
        import jax.random
        from IMP.algebra._jax_util import get_random_vector_in_3d_sphere
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
            v = get_random_vector_in_3d_sphere(subkey, radius)
            if intcoord:
                rbs = jm['rigid_bodies']
                rbs.intcoord = rbs.intcoord.at[indexes].add(v)
            else:
                jm['xyz'] = jm['xyz'].at[indexes].add(v)
            return jm, key, 1.0
        return self._wrap_jax(init_func, propose_func,
                              keys=['rigid_bodies'] if intcoord else None)
  %}
}

%extend IMP::core::RigidBodyMover {
  %pythoncode %{
    def _get_jax(self):
        import jax.random
        import IMP.core._jax_rigid
        from IMP.algebra._jax_util import (get_random_vector_on_3d_sphere,
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
                tf.translation += get_random_vector_in_3d_sphere(
                    subkey, max_translation)[0]
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
    def _get_jax(self):
        import jax.random
        import jax.lax
        import jax.numpy as jnp
        from IMP.core._jax_util import _SerialMover
        movers = [m.get_derived_object()._get_jax()
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
            keys=frozenset(x for m in movers for x in m._keys))
  %}
}

%extend IMP::core::MonteCarlo {
  %pythoncode %{
    def _get_jax(self):
        from IMP.core._jax_util import _MCJAXInfo
        return _MCJAXInfo(self)

    def _get_jax_optimizer(self, max_steps):
        import IMP.core._jax_util
        return IMP.core._jax_util._MCJAXOptimizer(self, max_steps)

    def _optimize_jax(self, max_steps):
        opt = self._get_jax_optimizer(max_steps)
        score, mc_state = opt.optimize(opt.get_initial_state())
        return score
  %}
}

%extend IMP::core::internal::_RigidBodyNormalizeConstraint {
  %pythoncode %{
    def _get_jax(self):
        from IMP.algebra._jax_util import _quaternion_normalize
        import jax
        normalize_rotations = jax.vmap(_quaternion_normalize, in_axes=0)
        def apply_func(jm):
            # Assume that constraint acts on all rigid bodies
            allrbs = jm['rigid_bodies']
            allrbs.quaternion = normalize_rotations(allrbs.quaternion)
            return jm
        return self._wrap_jax(apply_func, keys=['rigid_bodies'])
  %}
}

%extend IMP::core::internal::_RigidBodyPositionConstraint {
  %pythoncode %{
    def _get_jax(self):
        import jax
        def apply_func(jm):
            # Assume that constraint acts on all rigid bodies
            bodies = jm['rigid_bodies'].bodies
            # todo: this could perhaps be better parallelized, as in most
            # cases (at least, without nested rigid bodies) the update of
            # one rigid body does not affect members of another body
            for body in bodies:
                jm = body.update_members(jm)
            return jm
        return self._wrap_jax(apply_func, keys=['rigid_bodies'])
  %}
}

%extend IMP::core::NormalMover {
  %pythoncode %{
    def _get_jax(self):
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
    def _get_jax(self):
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
