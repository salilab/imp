%extend IMP::atom::MolecularDynamics {
  %pythoncode %{
    def _get_jax(self):
        from IMP.atom._jax_util import _MDJaxInfo
        return _MDJaxInfo(self)

    def _optimize_jax(self, max_steps):
        import IMP.atom._jax_util
        return IMP.atom._jax_util._md_optimize(self, max_steps)
  %}
}

%extend IMP::atom::ForceSwitch {
  %pythoncode %{
    def _get_jax(self):
        import jax.lax
        min_distance = self.get_min_distance()
        max_distance = self.get_max_distance()
        dist_dif = max_distance - min_distance
        value_prefactor = 1.0 / (dist_dif * dist_dif * dist_dif)
        deriv_prefactor = 6.0 * value_prefactor
        def mid_range_score(d):
            x = max_distance - d
            return (value_prefactor * x * x *
                    (max_distance + 2.0 * d - 3.0 * min_distance))
        def not_short_range_score(d):
            return jax.lax.cond(d > max_distance, lambda d: 0.0,
                                mid_range_score, d)
        def jax_func(score, d):
            factor = jax.lax.cond(d <= min_distance, lambda d: 1.0,
                                  not_short_range_score, d)
            return score * factor
        return jax_func
  %}
}

%extend IMP::atom::CoulombPairScore {
  %pythoncode %{
    def _get_jax(self):
        import math
        import jax
        import jax.numpy as jnp
        # 1 / (4pi * epsilon) * conversion factor to get score in kcal/mol
        # if distances are in angstroms
        avogadro = 6.02214179e23  # /mole
        electron_charge = 1.6021892e-19  # Coulomb
        permittivity_vacuum = 8.854187818e-12  # C/V/m
        kcal2joule = 4186.8
        factor = (avogadro * electron_charge * electron_charge *
                  1.0e10 / permittivity_vacuum / kcal2joule /
                  (4.0 * math.pi * self.get_relative_dielectric()))
        sf = self.get_smoothing_function().get_derived_object()
        # Function operates on a single distance + score; make it work on
        # an array instead using jax.vmap
        smoothing_function = jax.vmap(sf._get_jax())
        def jax_pair_score(X, indexes):
            xyzs = X['xyz'][indexes]
            qs = X['charge'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            scores = factor * jnp.prod(qs, axis=1) / drs
            return smoothing_function(scores, drs)
        return self._wrap_jax(jax_pair_score, keys=[Charged.get_charge_key()])
  %}
}

%extend IMP::atom::VelocityScalingOptimizerState {
  %pythoncode %{
    def _get_jax(self):
        import jax.lax
        import jax.numpy as jnp
        temperature = self.get_temperature()

        def scale_velocities(md, tkinetic):
            scale = jnp.sqrt(temperature / tkinetic)
            linvel = md.X['linvel'].at[md.simulation_indexes]
            md.X['linvel'] = linvel.multiply(scale)
            return md

        def apply_func(md):
            ekinetic = md.get_kinetic_energy()
            tkinetic = md.get_kinetic_temperature(ekinetic)
            return jax.lax.cond(tkinetic > 1e-8, scale_velocities,
                                lambda md, tk: md, md, tkinetic)
        return self._wrap_jax(lambda x: x, apply_func)
  %}
}
