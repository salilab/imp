%pythonbegin %{
  import functools
%}

%extend IMP::isd::UniformPrior {
  %pythoncode %{
    def _get_jax(self, space):
        import jax.numpy as jnp
        def score(jm, lb, ub, k, index):
            nuisance = jm['nuisance'][index]
            return 0.5 * k * (nuisance - jnp.clip(nuisance, lb, ub)) ** 2
        rng = self.get_range()
        f = functools.partial(score, lb=rng[0], ub=rng[1], k=self.get_k(),
                              index=self.get_index())
        return self._wrap_jax(f, keys=[Scale.get_scale_key()])
  %}
}

%extend IMP::isd::JeffreysRestraint {
  %pythoncode %{
    def _get_jax(self, space):
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
    def _get_jax(self, space):
        import jax.numpy as jnp
        import IMP._jax_util
        restraints = [r.get_derived_object() for r in self.restraints]
        multi_funcs, funcs, keys = IMP._jax_util.get_restraints_jax_funcs_keys(
            restraints, space)
        def jax_sf(jm):
            rets = [f(jm) for f in multi_funcs]
            if funcs:
                rets.append(jnp.stack([f(jm) for f in funcs], axis=-1))
            scores = jnp.concatenate(rets, axis=-1)
            return -jnp.sum(jnp.log(scores), axis=-1)
        return self._wrap_jax(jax_sf, keys=keys)
  %}
}

%extend IMP::isd::NuisanceScoreState {
  %pythoncode %{
    def _get_jax(self):
        import jax.lax
        import math
        import jax.numpy as jnp
        import IMP._jax_util
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
        upper_f, upper_p = n.get_has_upper_float(), n.get_has_upper_particle()
        lower_f, lower_p = n.get_has_lower_float(), n.get_has_lower_particle()
        if upper_p or lower_p:
            # These particle indexes can change during a simulation, so
            # we can't use the default CompactArray for nuisances (which
            # statically maps indexes beforehand)
            keys = [IMP._jax_util._DenseKey(n.get_nuisance_key())]
        else:
            keys = [n.get_nuisance_key()]
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

%extend IMP::isd::CrossLinkMSRestraint {
  %pythoncode %{
    @staticmethod
    def _get_contributions_sigma_numpy(rs):
        """Get the particle indexes for all sigma nuisances for all
           contributions to the given list of restraints."""
        import numpy
        import itertools
        nconts = [r.get_number_of_contributions() for r in rs]
        ret = numpy.empty((sum(nconts), 2), int)
        i = itertools.count(0)
        for (ncont, r) in zip(nconts, rs):
            for c in range(ncont):
                ret[next(i)] = r.get_contribution_sigma_indexes(c)
        return ret

    @staticmethod
    def _get_contributions_psi_numpy(rs):
        """Get the particle indexes for psi sigma nuisances for all
           contributions to the given list of restraints."""
        import numpy
        import itertools
        nconts = [r.get_number_of_contributions() for r in rs]
        ret = numpy.empty(sum(nconts), int)
        i = itertools.count(0)
        for (ncont, r) in zip(nconts, rs):
            for c in range(ncont):
                ret[next(i)] = r.get_contribution_psi_index(c)
        return ret

    @staticmethod
    def _get_contributions_particles_numpy(rs):
        """Get the particle indexes for crosslink endpoints for all
           contributions to the given list of restraints."""
        import numpy
        import itertools
        nconts = [r.get_number_of_contributions() for r in rs]
        ret = numpy.empty((sum(nconts), 2), int)
        i = itertools.count(0)
        for (ncont, r) in zip(nconts, rs):
            for c in range(ncont):
                ret[next(i)] = r.get_contribution_particle_indexes(c)
        return ret

    @staticmethod
    def _get_contributions_segments(rs):
        """Map each contribution to the corresponding restraint index,
           suitable for use by jax.ops.segment_prod()."""
        import numpy
        import itertools
        nconts = [r.get_number_of_contributions() for r in rs]
        # If every restraint has only a single contribution, skip
        # and return None; segment ids aren't needed
        if any(n > 1 for n in nconts):
            segs = []
            segment = itertools.count(0)
            for ncont in nconts:
                segs.extend([next(segment)] * ncont)
            return numpy.array(segs)

    @classmethod
    def _get_jax_for_restraints(cls, rs, space):
        """Get a JAX function that scores the given list of crosslink
           restraints. They must all have the same length, slope and
           get_log_prob flag."""
        import jax.numpy as jnp
        import jax.ops
        import math

        # 4-point Gauss-Legendre nodes and weights on [-1, 1], as in
        # get_forward_model_probability() in CrossLinkMSRestraint.cpp:
        # t = +-sqrt(3/7 + (2/7) sqrt(6/5)), w = (18 - sqrt(30)) / 36 and
        # t = +-sqrt(3/7 - (2/7) sqrt(6/5)), w = (18 + sqrt(30)) / 36
        # (Abramowitz & Stegun 1964, Table 25.4, p. 916).
        gl_x = jnp.array([-0.8611363115940526, -0.3399810435848563,
                          0.3399810435848563, 0.8611363115940526])
        gl_w = jnp.array([0.3478548451374538, 0.6521451548625461,
                          0.6521451548625461, 0.3478548451374538])

        def lens_volume(a, b, r):
            # Volume of the intersection of two balls of radii a and b
            # whose centers are r apart
            rmin = jnp.minimum(a, b)
            r_safe = jnp.where(r > 0., r, 1.)
            mid_range = (math.pi * (a + b - r) * (a + b - r)
                         * (r * r + 2. * r * (a + b) - 3. * (a - b) * (a - b))
                         / (12. * r_safe))
            return jnp.where(
                r >= a + b, 0.,
                jnp.where(r <= jnp.abs(a - b),
                          4. / 3. * math.pi * rmin * rmin * rmin, mid_range))

        def sphere_area_within(r, dist, length):
            # Area of the part of a sphere of radius r, centered at the
            # origin, within length of a point at distance dist
            mid_range = (math.pi * r * (length * length - (r - dist) ** 2)
                         / dist)
            outside = (r <= dist - length) | (r >= dist + length)
            return jnp.where(r + dist <= length, 4. * math.pi * r * r,
                             jnp.where(outside, 0., mid_range))

        def forward_model_probability(dist, sigmai, sigmaj, length):
            # See get_forward_model_probability() in CrossLinkMSRestraint.cpp
            rmax = sigmai + sigmaj
            points = jnp.stack([jnp.zeros_like(rmax), rmax,
                                jnp.abs(sigmai - sigmaj),
                                jnp.abs(dist - length), dist + length],
                               axis=-1)
            points = jnp.sort(jnp.clip(points, 0., rmax[..., None]), axis=-1)
            half = 0.5 * (points[..., 1:] - points[..., :-1])
            mid = 0.5 * (points[..., 1:] + points[..., :-1])
            r = mid[..., None] + half[..., None] * gl_x
            integrand = (lens_volume(sigmai[..., None, None],
                                     sigmaj[..., None, None], r)
                         * sphere_area_within(r, dist[..., None, None],
                                              length))
            integral = jnp.sum(half[..., None] * gl_w * integrand,
                               axis=(-2, -1))
            voli = 4.0 / 3.0 * math.pi * sigmai * sigmai * sigmai
            volj = 4.0 / 3.0 * math.pi * sigmaj * sigmaj * sigmaj
            return jnp.clip(integral / voli / volj, 0., 1.)

        def get_probability(xyz, r, scale, ps, sigma, psi, length,
                            slope, segments, numrsr):
            # If the residues are assigned to the same particle-domain
            # get the distance as if the residue positions were randomly
            # taken from within the sphere representing the domain
            # Lund O, Protein Eng. 1997 Nov;10(11):1241-8.
            dist = jnp.where(ps[:, 0] == ps[:, 1],
                             36.0 / 35.0 * r[ps[:, 0]],
                             space.distance(xyz[ps[:, 0]] - xyz[ps[:, 1]]))
            dist = jnp.maximum(dist, 0.0001)
            psi = scale[psi]
            sigmai = scale[sigma[:, 0]]
            sigmaj = scale[sigma[:, 1]]

            pofr = forward_model_probability(dist, sigmai, sigmaj, length)
            term = psi * (1.0 - pofr) + pofr * (1.0 - psi)
            if slope is not None:
                prior = jnp.exp(-slope * dist)
                term = term * prior
            onemprob = 1.0 - term

            if segments is None:
                # If only one contribution to each restraint, we are done
                return 1.0 - onemprob
            else:
                # Otherwise, take the product of all contributions to each
                # restraint
                return 1.0 - jax.ops.segment_prod(onemprob.T, segments, numrsr,
                                                  indices_are_sorted=True).T

        for r in rs:
            if r.get_is_length_variable():
                raise NotImplementedError("Only implemented for fixed-length")
        sigma = cls._get_contributions_sigma_numpy(rs)
        psi = cls._get_contributions_psi_numpy(rs)
        pis = cls._get_contributions_particles_numpy(rs)
        segments = cls._get_contributions_segments(rs)
        get_log_prob = rs[0].get_log_prob()
        length = rs[0].get_length()
        slope = rs[0].get_slope() if rs[0].get_has_slope() else None

        def jax_restraint(X):
            prob = get_probability(X['xyz'], X['r'], X['nuisance'], pis,
                                   sigma, psi, length, slope, segments,
                                   len(rs))
            if get_log_prob:
                return -jnp.log(prob)
            else:
                return prob

        # Note that we don't weight the restraint anywhere. This matches
        # the behavior of the C++ restraint.
        return cls._wrap_jax_multiple(rs[0], jax_restraint,
                                      keys=[IMP.isd.Scale.get_scale_key()])

    def _get_jax(self, space):
        return self._get_jax_for_restraints([self], space)

    @classmethod
    def _get_jax_multiple(cls, restraints, space):
        import IMP._jax_util
        # Get all groups of crosslink restraints that have the same length
        # and slope (to 4 decimal places) and get_log_prob flag
        def rsrkey(r):
            if type(r) is cls:
                return ("%.4f" % r.get_length(),
                        "%.4f" % r.get_slope() if r.get_has_slope() else None,
                        r.get_log_prob())
        groups, ungrouped = IMP._jax_util.get_grouped_restraints(
            restraints, rsrkey)

        return ([cls._get_jax_for_restraints(g, space) for g in groups],
                ungrouped)

  %}
}
