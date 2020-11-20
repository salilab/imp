/**
 *  \file IMP/atom/MolecularDynamics.h
 *  \brief Simple molecular dynamics optimizer.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MOLECULAR_DYNAMICS_H
#define IMPATOM_MOLECULAR_DYNAMICS_H

#include <IMP/atom/atom_config.h>
#include "Simulator.h"
#include "atom_macros.h"
#include <IMP/Particle.h>
#include <IMP/Optimizer.h>

IMPATOM_BEGIN_NAMESPACE

//! A particle with linear (XYZ) velocity
/** Typically this is used in combination with the MolecularDynamics optimizer.
 */
class IMPATOMEXPORT LinearVelocity : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                     const algebra::Vector3D v = algebra::Vector3D(0, 0, 0)) {
    m->add_attribute(get_velocities_key(), pi, v.get_coordinates());
  }
  static FloatsKey get_velocities_key() {
    static const FloatsKey key("linvel");
    return key;
  }

public:
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_velocities_key(), pi);
  }

  IMP_DECORATOR_METHODS(LinearVelocity, Decorator);
  IMP_DECORATOR_SETUP_0(LinearVelocity);
  IMP_DECORATOR_SETUP_1(LinearVelocity, algebra::Vector3D, v);

  void set_velocity(const algebra::Vector3D &v) {
    Model *m = get_model();
    ParticleIndex pi = get_particle_index();
    m->set_attribute(get_velocities_key(), pi, v.get_coordinates());
  }

  algebra::Vector3D get_velocity() const {
    Model *m = get_model();
    ParticleIndex pi = get_particle_index();
    return algebra::Vector3D(m->get_attribute(get_velocities_key(), pi));
  }
};

//! A particle with angular velocity
/** Typically this is used for RigidBody particles in combination
    with the MolecularDynamics optimizer. The velocity is stored as
    a quaternion.
 */
class IMPATOMEXPORT AngularVelocity : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                   const algebra::Vector4D v = algebra::Vector4D(0, 0, 0, 0)) {
    m->add_attribute(get_velocities_key(), pi, v.get_coordinates());
  }
  static FloatsKey get_velocities_key() {
    static const FloatsKey key("angvel");
    return key;
  }

public:

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_velocities_key(), pi);
  }

  IMP_DECORATOR_METHODS(AngularVelocity, Decorator);
  IMP_DECORATOR_SETUP_0(AngularVelocity);
  IMP_DECORATOR_SETUP_1(AngularVelocity, algebra::Vector4D, v);

  void set_velocity(const algebra::Vector4D &v) {
    Model *m = get_model();
    ParticleIndex pi = get_particle_index();
    m->set_attribute(get_velocities_key(), pi, v.get_coordinates());
  }

  algebra::Vector4D get_velocity() const {
    Model *m = get_model();
    ParticleIndex pi = get_particle_index();
    return algebra::Vector4D(m->get_attribute(get_velocities_key(), pi));
  }
};

//! Simple molecular dynamics optimizer.
/** The particles to be optimized must have optimizable x,y,z attributes
    and a non-optimizable mass attribute; this optimizer assumes the score
    to be energy in kcal/mol, the xyz coordinates to be in angstroms, and
    the mass to be in AMU (g/mol).

    \note RigidBody particles are not handled properly.

    Particles without optimized x,y,z and nonoptimized mass are skipped.
    \see VelocityScalingOptimizerState
    \see LangevinThermostatOptimizerState
    \see BerendsenThermostatOptimizerState
    \see RemoveRigidMotionOptimizerState
 */
class IMPATOMEXPORT MolecularDynamics : public Simulator {
 public:
  /** Score based on the provided model */
  MolecularDynamics(Model *m);

  //! Return the current kinetic energy of the system, in kcal/mol
  virtual Float get_kinetic_energy() const;

  //! Return the current kinetic temperature of the system
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
  virtual void setup(const ParticleIndexes &ps) IMP_OVERRIDE;
  virtual double do_step(const ParticleIndexes &sc,
                         double dt) IMP_OVERRIDE;
  virtual bool get_is_simulation_particle(ParticleIndex p) const
      IMP_OVERRIDE;

  IMP_OBJECT_METHODS(MolecularDynamics);

 protected:
  void initialize();

  virtual void setup_degrees_of_freedom(const ParticleIndexes &ps);

  //! First part of velocity Verlet (update coordinates and half-step velocity)
  virtual void propagate_coordinates(const ParticleIndexes &ps,
                                     double step_size);

  //! Second part of velocity Verlet (update velocity)
  virtual void propagate_velocities(const ParticleIndexes &ps,
                                    double step_size);

  //! Cap a velocity component to the maximum value.
  inline void cap_velocity_component(Float &vel) {
    if (vel >= 0.0) {
      vel = std::min(vel, velocity_cap_);
    } else {
      vel = std::max(vel, -velocity_cap_);
    }
  }

  //! Number of degrees of freedom in the system
  int degrees_of_freedom_;

  //! Maximum absolute value of a single velocity component
  Float velocity_cap_;
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_MOLECULAR_DYNAMICS_H */
