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
           suitable for use by IMP.isd._jax_util.get_segment_prod()."""
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
        import math

        def sphere_cap(r1, r2, d):
            short_range = jnp.minimum(4.0 / 3.0 * math.pi * r1 * r1 * r1,
                                      4.0 / 3.0 * math.pi * r2 * r2 * r2)
            is_short = d <= jnp.absolute(r1 - r2)
            is_long = d >= r1 + r2
            # Only evaluate the mid-range expression (which divides by d)
            # where it is actually used; otherwise d==0 in the short-range
            # branch yields NaN gradients even though the value is unused
            safe_d = jnp.where(is_short | is_long, 1., d)
            mid_range = ((math.pi / 12 / safe_d * (r1 + r2 - safe_d)
                          * (r1 + r2 - safe_d)) *
                         (safe_d * safe_d + 2 * safe_d * r1 - 3 * r1 * r1
                          + 2 * safe_d * r2 + 6 * r1 * r2 - 3 * r2 * r2))
            return jnp.where(is_short, short_range,
                             jnp.where(is_long, 0., mid_range))

        def forward_model(dist, sigmai, sigmaj, length):
            voli = 4.0 / 3.0 * math.pi * sigmai * sigmai * sigmai
            volj = 4.0 / 3.0 * math.pi * sigmaj * sigmaj * sigmaj
            xlvol = (4.0 / 3.0 * math.pi * (length / 2.)
                     * (length / 2.) * (length / 2.))

            close = dist < sigmai + sigmaj
            fi = jnp.where(
                close, jnp.minimum(voli, xlvol),
                sphere_cap(sigmai, length / 2.,
                           jnp.abs(dist - sigmaj - length / 2.)))
            fj = jnp.where(
                close, jnp.minimum(volj, xlvol),
                sphere_cap(sigmaj, length / 2.,
                           jnp.abs(dist - sigmai - length / 2.)))

            return fi * fj / voli / volj

        return _get_crosslink_jax_for_restraints(rs, space, forward_model)

    def _get_jax(self, space):
        return self._get_jax_for_restraints([self], space)

    @classmethod
    def _get_jax_multiple(cls, restraints, space):
        return _get_crosslink_jax_multiple(cls, restraints, space)
  %}
}

%extend IMP::isd::ExactCrossLinkMSRestraint {
  %pythoncode %{
    @classmethod
    def _get_jax_for_restraints(cls, rs, space):
        """Get a JAX function that scores the given list of crosslink
           restraints. They must all have the same length, slope and
           get_log_prob flag."""
        import jax.numpy as jnp
        import math

        # 4-point Gauss-Legendre nodes and weights on [-1, 1], as in
        # get_forward_model_probability() in ExactCrossLinkMSRestraint.cpp:
        # t = +-sqrt(3/7 + (2/7) sqrt(6/5)), w = (18 - sqrt(30)) / 36 and
        # t = +-sqrt(3/7 - (2/7) sqrt(6/5)), w = (18 + sqrt(30)) / 36
        # (Abramowitz & Stegun 1964, Table 25.4, p. 916).
        gl_x = jnp.array([-0.8611363115940526, -0.3399810435848563,
                          0.3399810435848563, 0.8611363115940526])
        gl_w = jnp.array([0.3478548451374538, 0.6521451548625461,
                          0.6521451548625461, 0.3478548451374538])

        def lens_volume(a, b, r):
            # Volume of the intersection of two balls of radii a and b
            # whose centers are r apart. Only divide by r where the
            # mid-range expression is used, so that gradients are finite.
            is_long = r >= a + b
            is_short = r <= jnp.abs(a - b)
            r_safe = jnp.where(is_long | is_short, a + b, r)
            rmin = jnp.minimum(a, b)
            mid_range = (math.pi * (a + b - r_safe) * (a + b - r_safe)
                         * (r_safe * r_safe + 2. * r_safe * (a + b)
                            - 3. * (a - b) * (a - b))
                         / (12. * r_safe))
            return jnp.where(
                is_long, 0.,
                jnp.where(is_short, 4. / 3. * math.pi * rmin * rmin * rmin,
                          mid_range))

        def sphere_area_within(r, dist, length):
            # Area of the part of a sphere of radius r, centered at the
            # origin, within length of a point at distance dist
            mid_range = (math.pi * r * (length * length - (r - dist) ** 2)
                         / dist)
            outside = (r <= dist - length) | (r >= dist + length)
            return jnp.where(r + dist <= length, 4. * math.pi * r * r,
                             jnp.where(outside, 0., mid_range))

        def forward_model(dist, sigmai, sigmaj, length):
            # See get_forward_model_probability() in
            # ExactCrossLinkMSRestraint.cpp. The integrand is continuous
            # in r and zero at the upper limit, so differentiating this
            # piecewise quadrature (with its moving breakpoints) gives the
            # exact derivatives.
            rmax = sigmai + sigmaj
            points = jnp.stack([jnp.zeros_like(rmax), rmax,
                                jnp.abs(sigmai - sigmaj),
                                jnp.abs(dist - length), dist + length],
                               axis=-1)
            points = jnp.sort(jnp.clip(points, 0., rmax[..., None]),
                              axis=-1)
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

        return _get_crosslink_jax_for_restraints(rs, space, forward_model)

    def _get_jax(self, space):
        return self._get_jax_for_restraints([self], space)

    @classmethod
    def _get_jax_multiple(cls, restraints, space):
        return _get_crosslink_jax_multiple(cls, restraints, space)
  %}
}

%extend IMP::isd::GaussianEMRestraint {
  %pythoncode %{
    def _get_jax(self, space):
        """Get a JAX implementation of this restraint.

           Unlike the C++ implementation, which uses close pair containers
           (which may contain some pairs up to the cutoff plus twice the
           slack), every model-model and model-density pair is considered
           here, and only those whose sphere distance is less than the
           cutoff are scored. Model Gaussian covariances are calculated
           from the current orientation of each Gaussian, so the score
           can also be differentiated with respect to orientation. Masses,
           variances, and density Gaussian covariances are assumed to
           be fixed.

           Periodic boundary conditions are not supported, since EM
           density maps are not periodic."""
        import numpy as np
        import jax
        import jax.numpy as jnp
        import IMP.jax
        import IMP.core
        import IMP.atom
        import IMP.core._jax_rigid
        from IMP.algebra.jax import _quaternion_to_rotation_matrix
        from IMP.isd._jax_util import get_gaussian_overlap

        if space is not IMP.jax.FreeSpace:
            raise NotImplementedError(
                "GaussianEMRestraint does not support periodic boundaries")

        m = self.get_model()

        def get_indexes(pis):
            return np.array([int(p) for p in pis], dtype=int)

        model_pis = self.get_model_particle_indexes()
        density_pis = self.get_density_particle_indexes()
        model_ps = get_indexes(model_pis)
        density_ps = get_indexes(density_pis)
        slope_ps = get_indexes(self.get_slope_particle_indexes())
        model_mass = np.array([IMP.atom.Mass(m, p).get_mass()
                               for p in model_pis])
        density_mass = np.array([IMP.atom.Mass(m, p).get_mass()
                                 for p in density_pis])
        model_var = np.array([IMP.core.Gaussian(m, p).get_variances()
                              for p in model_pis])
        density_covar = np.array(
            [np.asarray(IMP.core.Gaussian(m, p).get_global_covariance())
             for p in density_pis])
        # Each Gaussian is also a RigidBody; get the index into the
        # array of rigid body quaternions for each model Gaussian
        rb_from_particle = {
            int(p): i for (i, p) in enumerate(
                IMP.core._jax_rigid._get_rigid_body_indexes(m))}
        model_rbs = np.array([rb_from_particle[p] for p in model_ps],
                             dtype=int)
        md_mass = model_mass[:, np.newaxis] * density_mass[np.newaxis, :]
        # Only score each model-model pair once (the score is doubled
        # below). These pairs never change, so use static indexes rather
        # than a mask so that only the needed overlaps are calculated.
        mm_i, mm_j = np.triu_indices(len(model_ps), k=1)
        mm_mass = model_mass[mm_i] * model_mass[mm_j]
        model_cutoff = self.get_model_cutoff_distance()
        density_cutoff = self.get_density_cutoff_distance()
        slope = self.get_slope()
        self_mm_score = self.get_model_self_score()
        local = self.get_is_local()
        if local:
            # Precalculate all density-density overlaps; only those for
            # density particles near the model are used during scoring
            dxyz = np.array([IMP.core.XYZ(m, p).get_coordinates()
                             for p in density_pis])
            dd_matrix = get_gaussian_overlap(
                dxyz[np.newaxis, :] - dxyz[:, np.newaxis],
                density_covar[:, np.newaxis] + density_covar[np.newaxis, :],
                density_mass[:, np.newaxis] * density_mass[np.newaxis, :],
                xp=np)
        else:
            dd_score = self.get_density_density_score()

        def get_close_mask(dist, r1, r2, cutoff):
            """Get pairs whose sphere distance is less than cutoff"""
            return dist - r1[:, jnp.newaxis] - r2[jnp.newaxis, :] < cutoff

        def get_model_covariances(jm):
            quat = jm['rigid_bodies'].quaternion[model_rbs]
            # Note that this "rotation matrix" is the transpose of that
            # used in C++ (it is used to transform row vectors), so the
            # global covariance R diag(var) R^T is M^T diag(var) M
            rot = jax.vmap(_quaternion_to_rotation_matrix)(quat)
            return jnp.einsum('nji,nj,njk->nik', rot, model_var, rot)

        def jax_score(jm):
            xyz = jm['xyz']
            radius = jm['r']
            xm = xyz[model_ps]
            xd = xyz[density_ps]
            rm = radius[model_ps]
            rd = radius[density_ps]
            cm = get_model_covariances(jm)

            # Model-density overlap
            v_md = space.displacement(
                xd[jnp.newaxis, :] - xm[:, jnp.newaxis])
            ov_md = get_gaussian_overlap(
                v_md, cm[:, jnp.newaxis] + density_covar[jnp.newaxis, :],
                md_mass)
            md_mask = get_close_mask(space.distance(v_md), rm, rd,
                                     density_cutoff)
            md_score = jnp.sum(jnp.where(md_mask, ov_md, 0.))

            # Model-model overlap
            v_mm = space.displacement(xm[mm_j] - xm[mm_i])
            ov_mm = get_gaussian_overlap(v_mm, cm[mm_i] + cm[mm_j], mm_mass)
            mm_mask = (space.distance(v_mm) - rm[mm_i] - rm[mm_j]
                       < model_cutoff)
            mm_score = self_mm_score + 2. * jnp.sum(
                jnp.where(mm_mask, ov_mm, 0.))

            # Density-density overlap
            if local:
                # Only consider density particles near the model
                local_dens = jnp.any(md_mask, axis=0)
                dd = jnp.sum(jnp.where(
                    local_dens[:, jnp.newaxis] & local_dens[jnp.newaxis, :],
                    dd_matrix, 0.))
            else:
                dd = dd_score

            score = -jnp.log(2. * md_score / (mm_score + dd))
            if slope > 0.:
                # Clamp the distance so that the gradient is not NaN for
                # coincident model and density particles
                dist = space.distance(
                    xyz[slope_ps][:, jnp.newaxis] - xd[jnp.newaxis, :],
                    min_dist=1e-8)
                score = score + slope * jnp.sum(dist)
            return score

        return self._wrap_jax(jax_score, keys=['rigid_bodies'])
  %}
}

%pythoncode %{
def _get_crosslink_jax_for_restraints(rs, space, forward_model):
    """Get a JAX function that scores the given list of crosslink
       restraints (CrossLinkMSRestraint or ExactCrossLinkMSRestraint),
       using the given forward model. They must all have the same length,
       slope and get_log_prob flag."""
    import jax.numpy as jnp

    def get_probability(xyz, r, scale, ps, sigma, psi, length,
                        slope, segment_prod):
        # If the residues are assigned to the same particle-domain
        # get the distance as if the residue positions were randomly
        # taken from within the sphere representing the domain
        # Lund O, Protein Eng. 1997 Nov;10(11):1241-8.
        # Let the space clamp the distance, so that the gradient is
        # zero, rather than NaN, for coincident points and for
        # same-particle contributions (which don't use the distance)
        dist = jnp.where(
            ps[:, 0] == ps[:, 1], 36.0 / 35.0 * r[ps[:, 0]],
            space.distance(xyz[ps[:, 0]] - xyz[ps[:, 1]],
                           min_dist=0.0001))
        dist = jnp.maximum(dist, 0.0001)
        psi = scale[psi]
        sigmai = scale[sigma[:, 0]]
        sigmaj = scale[sigma[:, 1]]

        pofr = forward_model(dist, sigmai, sigmaj, length)
        term = psi * (1.0 - pofr) + pofr * (1.0 - psi)
        if slope is not None:
            prior = jnp.exp(-slope * dist)
            term = term * prior
        onemprob = 1.0 - term

        if segment_prod is None:
            # If only one contribution to each restraint, we are done
            return 1.0 - onemprob
        else:
            # Otherwise, take the product of all contributions to each
            # restraint
            return 1.0 - segment_prod(onemprob.T).T

    for r in rs:
        if r.get_is_length_variable():
            raise NotImplementedError("Only implemented for fixed-length")
    sigma = CrossLinkMSRestraint._get_contributions_sigma_numpy(rs)
    psi = CrossLinkMSRestraint._get_contributions_psi_numpy(rs)
    pis = CrossLinkMSRestraint._get_contributions_particles_numpy(rs)
    segments = CrossLinkMSRestraint._get_contributions_segments(rs)
    if segments is None:
        segment_prod = None
    else:
        # jax.ops.segment_prod is not differentiable, so use our own
        from IMP.isd._jax_util import get_segment_prod
        segment_prod = get_segment_prod(segments, len(rs))
    get_log_prob = rs[0].get_log_prob()
    length = rs[0].get_length()
    slope = rs[0].get_slope() if rs[0].get_has_slope() else None

    def jax_restraint(X):
        prob = get_probability(X['xyz'], X['r'], X['nuisance'], pis,
                               sigma, psi, length, slope, segment_prod)
        if get_log_prob:
            return -jnp.log(prob)
        else:
            return prob

    # Note that we don't weight the restraint anywhere. This matches
    # the behavior of the C++ restraint.
    return rs[0]._wrap_jax_multiple(rs[0], jax_restraint,
                                    keys=[Scale.get_scale_key()])


def _get_crosslink_jax_multiple(cls, restraints, space):
    """Get JAX functions for all restraints of type cls in the list"""
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
