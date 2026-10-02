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
        funcs, keys = self._get_restraint_jax_funcs_keys(space)
        def jax_sf(jm):
            scores = jnp.asarray([f(jm) for f in funcs])
            return -jnp.sum(jnp.log(scores))
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
    def _get_contributions_sigma_numpy(self):
        import numpy
        n = self.get_number_of_contributions()
        ret = numpy.empty((n, 2), int)
        for i in range(n):
            ret[i] = self.get_contribution_sigma_indexes(i)
        return ret

    def _get_contributions_psi_numpy(self):
        import numpy
        n = self.get_number_of_contributions()
        ret = numpy.empty(n, int)
        for i in range(n):
            ret[i] = self.get_contribution_psi_index(i)
        return ret

    def _get_contributions_particles_numpy(self):
        import numpy
        n = self.get_number_of_contributions()
        ret = numpy.empty((n, 2), int)
        for i in range(n):
            ret[i] = self.get_contribution_particle_indexes(i)
        return ret

    def _get_jax(self, space):
        import jax.numpy as jnp
        import math

        def sphere_cap(r1, r2, d):
            short_range = jnp.minimum(4.0 / 3.0 * math.pi * r1 * r1 * r1,
                                      4.0 / 3.0 * math.pi * r2 * r2 * r2)
            mid_range = ((math.pi / 12 / d * (r1 + r2 - d) * (r1 + r2 - d)) *
                         (d * d + 2 * d * r1 - 3 * r1 * r1 + 2 * d * r2
                          + 6 * r1 * r2 - 3 * r2 * r2))
            return jnp.where(
                d <= jnp.absolute(r1 - r2), short_range,
                jnp.where(d >= r1 + r2, 0., mid_range))

        def get_probability(xyz, r, scale, ps, sigma, psi, length,
                            slope):
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

            pofr = fi * fj / voli / volj
            term = psi * (1.0 - pofr) + pofr * (1.0 - psi)
            if slope is not None:
                prior = jnp.exp(-slope * dist)
                term = term * prior
            onemprob = 1.0 - term

            return 1.0 - jnp.prod(onemprob)

        if self.get_is_length_variable():
            raise NotImplementedError("Only implemented for fixed-length")
        sigma = self._get_contributions_sigma_numpy()
        psi = self._get_contributions_psi_numpy()
        pis = self._get_contributions_particles_numpy()
        get_log_prob = self.get_log_prob()
        length = self.get_length()
        slope = self.get_slope() if self.get_has_slope() else None

        def jax_restraint(X):
            prob = get_probability(X['xyz'], X['r'], X['nuisance'], pis,
                                   sigma, psi, length, slope)
            if get_log_prob:
                return -jnp.log(prob)
            else:
                return prob

        return self._wrap_jax(jax_restraint,
                              keys=[IMP.isd.Scale.get_scale_key()])
  %}
}
