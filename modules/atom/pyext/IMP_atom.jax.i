%pythonbegin %{
  import functools
%}

%extend IMP::atom::MolecularDynamics {
  %pythoncode %{
    def _get_jax(self):
        from IMP.atom._jax_util import _MDJAXInfo
        return _MDJAXInfo(self)

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
    def _get_jax(self, m, indexes):
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
        def score(jm):
            xyzs = jm['xyz'][indexes]
            qs = jm['charge'][indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            scores = factor * jnp.prod(qs, axis=1) / drs
            return smoothing_function(scores, drs)
        return self._wrap_jax(m, score, keys=[Charged.get_charge_key()])
  %}
}

%extend IMP::atom::DopePairScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import numpy as np
        import jax.lax
        import jax.numpy as jnp
        from IMP.core._jax_util import _spline
        def score_one(jm, index, minrange, maxrange, spacing, values,
                      second_derivs):
            # Score a single atom pair
            xyz = jnp.asarray(jm['xyz'])[index]
            dr = jnp.linalg.norm(xyz[1] - xyz[0])
            def spline_score():
                dope_type_pair = jnp.asarray(jm['dope atom type'])
                # Get index into values/second_derivs tables
                i = jnp.max(dope_type_pair[index])
                j = jnp.min(dope_type_pair[index])
                dope_type = i * (i + 1) // 2 + j
                # tabulated values are for the center of each bin, but
                # _spline works with the values at the low side of each bin
                x = dr - 0.5 * spacing
                # determine bin index and thus the cubic fragment to use:
                lowbin = jnp.array((x - minrange) / spacing, dtype=int)
                return _spline(x, minrange, lowbin, lowbin + 1, spacing,
                               values[dope_type], second_derivs[dope_type])
            def maybe_score():
                return jax.lax.cond(dr <= minrange, lambda: 0.0, spline_score)
            return jax.lax.cond(dr >= maxrange, lambda: 0.0, maybe_score)
        sf = self.get_score_functor()
        if sf.get_is_bipartite() or not sf.get_is_interpolated():
            raise TypeError("Only implemented for unipartite splines")
        ntyp = sf.get_number_of_particle_types()[0]
        # Get spline parameters for all atom types. The matrix is symmetric
        # so only store the upper diagonal
        values = []
        second_derivs = []
        for i in range(ntyp):
            for j in range(0, i + 1):
                values.append(sf.get_values(i, j))
                second_derivs.append(sf.get_second_derivatives(i, j))
        values = np.vstack(values)
        second_derivs = np.vstack(second_derivs)
        f = functools.partial(
            score_one, minrange=sf.get_offset(),
            maxrange=min(sf.get_max(), sf.get_threshold()),
            spacing=sf.get_spacing(), values=jnp.asarray(values),
            second_derivs=jnp.asarray(second_derivs))
        # Vectorize to take multiple indexes (second argument)
        vf = jax.vmap(f, in_axes=(None, 0))
        return self._wrap_jax(m, lambda jm: vf(jm, indexes),
                              keys=(sf.get_dope_type_key(),))
  %}
}

%extend IMP::atom::VelocityScalingOptimizerState {
  %pythoncode %{
    def _get_jax(self, state_index):
        import jax.lax
        import jax.numpy as jnp
        temperature = self.get_temperature()

        def scale_velocities(md, tkinetic):
            scale = jnp.sqrt(temperature / tkinetic)
            linvel = md.jm['linvel'].at[md.simulation_indexes]
            md.jm['linvel'] = linvel.multiply(scale)
            return md

        def apply_func(md):
            ekinetic = md.get_kinetic_energy()
            tkinetic = md.get_kinetic_temperature(ekinetic)
            return jax.lax.cond(tkinetic > 1e-8, scale_velocities,
                                lambda md, tk: md, md, tkinetic)
        return self._wrap_jax(lambda x: x, apply_func)
  %}
}

%extend IMP::atom::BondSingletonScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        from IMP.atom._jax_util import _get_bonds
        def score(jm, bonds, uf):
            xyzs = jm['xyz'][bonds.bonded_indexes]
            diff = xyzs[:,0] - xyzs[:,1]
            drs = jnp.linalg.norm(diff, axis=1)
            return uf(bonds.stiffness * (drs - bonds.length))
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, bonds=_get_bonds(m, indexes),
                              uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::atom::AngleSingletonScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        import IMP.core._jax_util
        from IMP.atom._jax_util import _get_angles
        def score(jm, angles, uf):
            xyzs = jm['xyz'][angles.bonded_indexes]
            rij = xyzs[:,0] - xyzs[:,1]
            rkj = xyzs[:,2] - xyzs[:,1]
            angle = IMP.core._jax_util._angle(rij, rkj)
            angle_diff = IMP.core._jax_util._get_angle_difference(
                angle, angles.ideal)
            return uf(angles.stiffness * angle_diff)
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, angles=_get_angles(m, indexes),
                              uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::atom::DihedralSingletonScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        import IMP.core._jax_util
        from IMP.atom._jax_util import _get_dihedrals
        def score(jm, dihedrals):
            xyzs = jm['xyz'][dihedrals.bonded_indexes]
            rij = xyzs[:,0] - xyzs[:,1]
            rkj = xyzs[:,2] - xyzs[:,1]
            rkl = xyzs[:,2] - xyzs[:,3]
            dihedral = IMP.core._jax_util._dihedral(rij, rkj, rkl)
            b = 0.5 * dihedrals.stiffness * jnp.abs(dihedrals.stiffness)
            return jnp.abs(b) + b * jnp.cos(dihedral * dihedrals.multiplicity
                                            + dihedrals.ideal)
        f = functools.partial(score, dihedrals=_get_dihedrals(m, indexes))
        return self._wrap_jax(m, f)
  %}
}

%extend IMP::atom::ImproperSingletonScore {
  %pythoncode %{
    def _get_jax(self, m, indexes):
        import jax.numpy as jnp
        import IMP.core._jax_util
        from IMP.atom._jax_util import _get_dihedrals
        def score(jm, dihedrals, uf):
            xyzs = jm['xyz'][dihedrals.bonded_indexes]
            rij = xyzs[:,0] - xyzs[:,1]
            rkj = xyzs[:,2] - xyzs[:,1]
            rkl = xyzs[:,2] - xyzs[:,3]
            dihedral = IMP.core._jax_util._dihedral(rij, rkj, rkl)
            angle_diff = IMP.core._jax_util._get_angle_difference(
                dihedral, dihedrals.ideal)
            return uf(dihedrals.stiffness * angle_diff)
        uf = self.get_unary_function().get_derived_object()
        f = functools.partial(score, dihedrals=_get_dihedrals(m, indexes),
                              uf=uf._get_jax())
        return self._wrap_jax(m, f)
  %}
}
