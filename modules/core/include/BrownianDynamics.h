/**
 *  \file BrownianDynamics.h    \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_BROWNIAN_DYNAMICS_H
#define IMPCORE_BROWNIAN_DYNAMICS_H

#include "config.h"
#include "internal/version_info.h"
#include "DiffusionDecorator.h"

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Simple Brownian dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable "Stokes radius"; this optimizer assumes
    the score to be energy in kcal/mol, the xyz coordinates to be in
    angstroms and the diffusion coefficent be in cm^2/s

    Particles without optimized x,y,z and nonoptimized D are skipped.
    \deprecated Use atom::BrownianDynamics instead
    \see DiffusionDecorator
  */
class IMPCOREEXPORT BrownianDynamics : public Optimizer
{
public:
  //! Create the optimizer
  BrownianDynamics();
  virtual ~BrownianDynamics();

  IMP_OPTIMIZER(internal::version_info);

  //! Simulate until the given time in fs
  void simulate(float time_in_fs);

  //! Set time step in fs
  void set_time_step_in_femtoseconds(Float t) {
    set_time_step(unit::Femtosecond(t));
  }

  //! Time step in fs
  Float get_time_step_in_femtoseconds() const {
    return get_time_step_units().get_value();
  }

  /** In Kelvin*/
  void set_temperature_in_kelvin(Float t) { set_temperature(unit::Kelvin(t)); }
  /** In Kelvin */
  Float get_temperature_in_kelvin() const {
    return get_temperature_units().get_value();
  }

  //! Estimate the radius of a protein from the mass
  /** Proteins are assumed to be spherical. The density is estimated
      using the formula from
      <http://www.proteinscience.org/cgi/content/full/13/10/2825>

      The formula is:
      density= 1.410+ 0.145 exp(-M/13) g/cm^3
   */
  static Float estimate_radius_from_mass(Float mass_in_kd) {
    return
 estimate_radius_from_mass_units(unit::Kilodalton(mass_in_kd)).get_value();
  }

  //! Return the expected distance moved for a particle with a given D
  /** The units on D are cm^2/sec and the return has units of Angstroms.
   */
  Float compute_sigma_from_D(Float D) const {
    unit::SquareCentimeterPerSecond du(D);
    return compute_sigma_from_D(du).get_value();
  }

  //! Returns a force value which would move the particle by sigma
  /** This value is in KCal/A/mol
   */
  Float get_force_scale_from_D(Float D) const {
    unit::SquareCentimeterPerSecond du(D);
    return get_force_scale_from_D(du).get_value();
  }

  //! Get the current time in femtoseconds
  Float get_current_time_in_femtoseconds() const {
    return get_current_time_units().get_value();
  }

  //! Set the current time in femtoseconds
  void set_current_time_in_femptoseconds(Float fs) {
    set_current_time( unit::Femtosecond(fs));
  }

  IMP_LIST(private, Particle, particle, Particle*);

private:

  unit::Femtosecond get_time_step_units() const {return dt_;}

  void take_step();

  unit::Femtojoule kt() const;

  void setup_particles();

  void set_time_step(unit::Femtosecond t);

  void set_temperature(unit::Kelvin t) { T_=t;}

  unit::Kelvin get_temperature_units() const { return T_;}

  static unit::Angstrom
    estimate_radius_from_mass_units(unit::Kilodalton mass_in_kd);

  unit::Angstrom
    compute_sigma_from_D(unit::SquareCentimeterPerSecond D) const;

  unit::KilocaloriePerAngstromPerMol
    get_force_scale_from_D(unit::SquareCentimeterPerSecond D) const;

  unit::Femtosecond get_current_time_units() const {
    return cur_time_;
  }

  //! Set the current time in femtoseconds
  void set_current_time(unit::Femtosecond fs) {
    cur_time_= fs;
  }

  unit::Femtosecond dt_, cur_time_;
  unit::Kelvin T_;
};

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BROWNIAN_DYNAMICS_H */
