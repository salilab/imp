/**
 *  \file BrownianDynamics.h    \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BROWNIAN_DYNAMICS_H
#define __IMP_BROWNIAN_DYNAMICS_H

#include "../IMP_config.h"
#include "../Particle.h"
#include "../Optimizer.h"
#include "../internal/units.h"
#include "../internal/kernel_version_info.h"
#include "../Vector3D.h"

namespace IMP
{

//! Simple Brownian dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable "Stokes radius"; this optimizer assumes 
    the score to be energy in kcal/mol, the xyz coordinates to be in
    angstroms and the diffusion coefficent be in cm^2/s

    Particles without optimized x,y,z and nonoptimized D are skipped.

    \todo Currently dynamic time steps are handled by scaling back dt
    if forces are too high at the current position. It would be better
    to scale back the time step if the forces will be too high at the
    next position.

    \ingroup optimizer
 */
class IMPDLLEXPORT BrownianDynamics : public Optimizer
{
public:
  //! \param[in] radkey The key for the Stokes radius
  BrownianDynamics(FloatKey dkey= FloatKey("D"));
  virtual ~BrownianDynamics();

  IMP_OPTIMIZER(internal::kernel_version_info)

  //! Set the max step that should be allowed in angstroms
  /** The timestep is shrunk if any particles are moved more than this */
  void set_max_step(Float a) {
    set_max_step(internal::Angstrom(a));
  }

  //! Set time step in fs
  void set_time_step(Float t) {
    set_time_step(internal::FemtoSecond(t));
  }

  //! Time step in fs
  Float get_time_step() const {
    return get_time_step_units().get_value();
  }

  //! The last time step used in fs
  Float get_last_time_step() const {
    return get_last_time_step_units().get_value();
  }


  void set_temperature(Float t) { set_temperature(internal::Kelvin(t)); }
  Float get_temperature() const { return get_temperature_units().get_value(); }

  FloatKey get_d_key() const { return dkey_; }

  //! Estimate the diffusion coeffient from the mass in kD
  /** This method does not do anything fancy, but is OK for large
      globular proteins.

      Note that it depends on temperature and so can't be static.
   */
  Float estimate_radius_from_mass(Float mass_in_kd) const {
    return estimate_radius_from_mass_units(mass_in_kd).get_value();
  }

  //! Return the expected distance moved for a particle with a given D
  /** The units on D are cm^2/sec and the return has units of Angstroms.
   */
  Float compute_sigma_from_D(Float D) const {
    return compute_sigma_from_D(internal::Cm2PerSecond(D)).get_value();
  }

  //! Estimate the diffusion coefficient from the radius in Angstroms
  /** This depends on the temperature.
   */
  Float estimate_D_from_radius(Float radius_in_angstroms) const {
    internal::Angstrom r(radius_in_angstroms);
    return estimate_D_from_radius(r).get_value();
  }

  //! Returns a force value which would move the particle by sigma
  /** This value is in KCal/A/mol
   */
  Float get_force_scale_from_D(Float D) const {
    return get_force_scale_from_D(internal::Cm2PerSecond(D)).get_value();
  }

  //! Get the current time in femtoseconds
  Float get_current_time() const {
    return get_current_time_units().get_value();
  }

  //! Set the current time in femtoseconds
  void set_current_time(Float fs) {
    set_current_time( internal::FemtoSecond(fs));
  }

  //! Return a histogram of timesteps
  /** The 0 value is the bin from get_time_step() to get_time_step()/2
      and they go as factors of two from there.
   */
  const std::vector<int> get_time_step_histogram() const {
    return time_steps_;
  }

  IMP_LIST(private, Particle, particle, Particle*);

protected:
  //! Perform a single dynamics step.
  //  \return true if the initial step size was OK
  bool step();

  /** Propose a single step, this will be accepted if all moves are
      small enough.
      \return true if it should be accepted.
   */
  bool propose_step(std::vector<Vector3D> &proposal);

  internal::FemtoJoule kt() const;

  void setup_particles();

  void set_max_step(internal::Angstrom a) {
    max_change_= a;
  }

  void set_time_step(internal::FemtoSecond t);

  internal::FemtoSecond get_time_step_units() const {return max_dt_;}

  internal::FemtoSecond get_last_time_step_units() const {return cur_dt_;}

  void set_temperature(internal::Kelvin t) { T_=t;}

  internal::Kelvin get_temperature_units() const { return T_;}

  internal::Angstrom estimate_radius_from_mass_units(Float mass_in_kd) const;

  internal::Angstrom compute_sigma_from_D(internal::Cm2PerSecond D) const;

  internal::Cm2PerSecond
  estimate_D_from_radius(internal::Angstrom radius) const;

  internal::KCalPerAMol get_force_scale_from_D(internal::Cm2PerSecond D) const;

  internal::FemtoSecond get_current_time_units() const {
    return cur_time_;
  }

  //! Set the current time in femtoseconds
  void set_current_time(internal::FemtoSecond fs) {
    cur_time_= fs;
  }

  internal::Angstrom max_change_;
  internal::FemtoSecond max_dt_, cur_dt_, cur_time_;
  internal::Kelvin T_;

  FloatKey dkey_;

  std::vector<int> time_steps_;
};

} // namespace IMP

#endif  /* __IMP_BROWNIAN_DYNAMICS_H */
