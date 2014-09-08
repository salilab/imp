/**
 *  \file membrane/MolecularDynamicsWithWte.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MOLECULAR_DYNAMICS_WITH_WTE_H
#define IMPMEMBRANE_MOLECULAR_DYNAMICS_WITH_WTE_H

#include "membrane_config.h"
#include "IMP/atom/Simulator.h"
#include "IMP/atom/atom_macros.h"
#include <IMP/Particle.h>
#include <IMP/Optimizer.h>
#include <boost/scoped_array.hpp>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Simple molecular dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable mass attribute; this optimizer assumes the score
    to be energy in kcal/mol, the xyz coordinates to be in angstroms, and
    the mass to be in AMU (g/mol).

    Particles without optimized x,y,z and nonoptimized mass are skipped.
    \see VelocityScalingOptimizerState
    \see LangevinThermostatOptimizerState
    \see BerendsenThermostatOptimizerState
    \see RemoveRigidMotionOptimizerState
 */
class IMPMEMBRANEEXPORT MolecularDynamicsWithWte : public atom::Simulator
{
private:
  double  min_, max_, sigma_, gamma_, dx_, w0_, currentscore_;
  boost::scoped_array<double> bias_;
  int     nbin_;
  void    update_bias(double score);
  double  get_derivative(double score) const;
  double  deriv_to_acceleration_;

public:
  /** Score based on the provided model */
  MolecularDynamicsWithWte(Model *m, double emin, double emax,
                           double sigma, double gamma, double w0);

  double get_bias(double score) const;

  Floats get_bias_buffer() const {
    Floats  buffer(bias_.get(), bias_.get()+2*nbin_);
   return buffer;
  }

  int get_nbin() const {
   return nbin_;
  }

  void set_w0(double w0) {w0_=w0;}

  void set_bias(const Floats& bias) {
   IMP_USAGE_CHECK(static_cast<int>(bias.size()) == 2*nbin_, "Don't match");
   // Getting over a warning about comparing unsigned to signed int.
   // It wont work if x >= INT_MIN (max integer size)
   std::copy(bias.begin(), bias.end(), bias_.get());
  }

  //! \return the current kinetic energy of the system, in kcal/mol
  virtual Float get_kinetic_energy() const;

  //! \return the current kinetic temperature of the system
  /** \param[in] ekinetic kinetic energy, e.g. from get_kinetic_energy()
   */
  Float get_kinetic_temperature(Float ekinetic) const;

  //! Set maximum velocity in A/fs
  /** At each dynamics time step, the absolute value of each velocity
      component is capped at this value. This prevents spurious strong forces
      (occasionally encountered with frustrated conformations) from causing
      large oscillations in the system.
      By default, velocities are not capped.

      \note The actual velocities that are capped are the half-step velocities
            in the velocity Verlet algorithm.
   */
  void set_velocity_cap(Float velocity_cap) { velocity_cap_ = velocity_cap; }

  //! Assign velocities representative of the given temperature
  virtual void assign_velocities(Float temperature);

  //! Rescale velocities
  void rescale_velocities(Float rescale);

  //IMP_SIMULATOR(MolecularDynamicsWithWte);
  virtual void setup(const ParticleIndexes &ps);
  virtual double do_step(const ParticleIndexes &sc, double dt);
  virtual bool get_is_simulation_particle(ParticleIndex p) const;

protected:
  void initialize();

  virtual void setup_degrees_of_freedom(const ParticleIndexes &ps);

  //! First part of velocity Verlet (update coordinates and half-step velocity)
  virtual void propagate_coordinates(const ParticleIndexes &ps, double
          step_size);

  //! Second part of velocity Verlet (update velocity)
  virtual void propagate_velocities(const ParticleIndexes &ps, double
          step_size);

  //! Cap a velocity component to the maximum value.
  inline void cap_velocity_component(Float &vel) {
    if (vel >= 0.0) {
      vel = std::min(vel, velocity_cap_);
    } else {
      vel = std::max(vel, -velocity_cap_);
    }
  }

  //! Keys of the xyz velocities
  FloatKey vs_[3];

  //! Number of degrees of freedom in the system
  int degrees_of_freedom_;

  //! Maximum absolute value of a single velocity component
  Float velocity_cap_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MOLECULAR_DYNAMICS_WITH_WTE_H */
