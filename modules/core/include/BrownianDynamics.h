/**
 *  \file BrownianDynamics.h    \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_BROWNIAN_DYNAMICS_H
#define IMPCORE_BROWNIAN_DYNAMICS_H

#include "config.h"
#include "internal/core_version_info.h"
#include "DiffusionDecorator.h"

#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <IMP/internal/units.h>
#include <IMP/algebra/Vector3D.h>

IMPCORE_BEGIN_NAMESPACE

//! Simple Brownian dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable "Stokes radius"; this optimizer assumes
    the score to be energy in kcal/mol, the xyz coordinates to be in
    angstroms and the diffusion coefficent be in cm^2/s

    Particles without optimized x,y,z and nonoptimized D are skipped.

    \relates DiffusionDecorator
  */
class IMPCOREEXPORT BrownianDynamics : public Optimizer
{
public:
  //! Create the optimizer
  BrownianDynamics();
  virtual ~BrownianDynamics();

  IMP_OPTIMIZER(internal::core_version_info);

  //! Simulate until the given time in fs
  void simulate(float time_in_fs);

  //! Set the max step that should be allowed in angstroms
  /** The timestep is shrunk if any particles are moved more than this */
  void set_max_step(Float a) {
    set_max_step(unit::Angstrom(a));
  }

  //! Set time step in fs
  void set_time_step(Float t) {
    set_time_step(unit::Femtosecond(t));
  }

  //! Time step in fs
  Float get_time_step() const {
    return get_time_step_units().get_value();
  }

  //! The last time step used in fs
  Float get_last_time_step() const {
    return get_last_time_step_units().get_value();
  }

  /** In Kelvin*/
  void set_temperature(Float t) { set_temperature(unit::Kelvin(t)); }
  /** In Kelvin */
  Float get_temperature() const { return get_temperature_units().get_value(); }

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
  Float get_current_time() const {
    return get_current_time_units().get_value();
  }

  //! Set the current time in femtoseconds
  void set_current_time(Float fs) {
    set_current_time( unit::Femtosecond(fs));
  }

  //! Return a histogram of timesteps
  /** The 0 value is the bin from get_time_step() to get_time_step()/2
      and they go as factors of two from there.
   */
  const std::vector<int> get_time_step_histogram() const {
    return time_steps_;
  }

  IMP_LIST(private, Particle, particle, Particle*);

private:

  //! Perform a single dynamics step.
  //  \return true if the initial step size was OK
  bool step();

  void take_step();

  /** Propose a single step, this will be accepted if all moves are
      small enough.
      \return true if it should be accepted.
   */
  bool propose_step(std::vector<algebra::Vector3D> &proposal);

  unit::Femtojoule kt() const;

  void setup_particles();

  void set_max_step(unit::Angstrom a) {
    max_change_= a;
  }

  void set_time_step(unit::Femtosecond t);

  unit::Femtosecond get_time_step_units() const {return max_dt_;}

  unit::Femtosecond get_last_time_step_units() const {return cur_dt_;}

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

  unsigned int num_const_dt_;
  unit::Angstrom max_change_;
  unit::Femtosecond max_dt_, cur_dt_, cur_time_;
  unit::Kelvin T_;

  std::vector<int> time_steps_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BROWNIAN_DYNAMICS_H */
