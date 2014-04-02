/**
 *  \file IMP/atom/TAMDParticle.h
 *  \brief A decorator for a diffusing particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_TAMD_PARTICLE_H
#define IMPATOM_TAMD_PARTICLE_H

#include <IMP/atom/atom_config.h>

#include <IMP/Decorator.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/kernel/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a diffusing particle that is simulated in a TAMD
//! BD simulation.
/** \ingroup helper
    \ingroup decorators
    \see BrownianDynamicsBarak

    TAMDParticle is used to decorate diffusing particle that are
    simulated as part of a TAMD simulation (Temperature-Accelerated
    MD), and exhibit elevated temperature and elevated friction
    coefficient (=actual diffusion coefficient reduced during TAMD BD
    simulation).

    See: Maragliano and Vanden-Eijden, 2006;
         Abrams and Vanden-Eijden, PNAS 2011
 */
class IMPATOMEXPORT TAMDParticle : public IMP::Decorator  {
  /**
     Setup a TAMD particle with simulation temperature elevated by a factor
     tsf, and friction coefficient elevated by a factor tsf (actual diffusion
     coefficient reduced by
  */
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                  float tsf=1.0, float fsf=1.0);

 public:
  IMP_DECORATOR_METHODS(TAMDParticle, IMP::Decorator);
  IMP_DECORATOR_SETUP_2(TAMDParticle, Float, tsf, Float, fsf);

  //! Return true if the particle is an instance of an TAMDParticle
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex p) {
    return m->get_has_attribute(get_temperature_scale_factor_key(), p);
    return m->get_has_attribute(get_friction_scale_factor_key(), p);
  }

  //! set temperature factoring for particle relative to simulation
  //! temperature
  void set_temperature_scale_factor(float tsf) {
    get_particle()->set_value(get_temperature_scale_factor_key(), tsf);
  }

  //! get temperature factoring for particle relative to simulation
  //! temperature
  double get_temperature_scale_factor() const {
    return get_particle()->get_value(get_temperature_scale_factor_key());
  }

  //! set friction factoring for particle relative to particle
  //! diffusion coefficient / rotational diffusion coefficient
  void set_friction_scale_factor(float tsf) {
    get_particle()->set_value(get_friction_scale_factor_key(), tsf);
  }

  //! get friction factoring for particle relative to particle
  //! diffusion coefficient / rotational diffusion coefficient
  double get_friction_scale_factor() const {
    return get_particle()->get_value(get_friction_scale_factor_key());
  }

  //! Get the temperature scale factor key
  static FloatKey get_temperature_scale_factor_key();

  //! Get the temperature scale factor key
  static FloatKey get_friction_scale_factor_key();

};

IMP_DECORATORS(TAMDParticle, TAMDParticles, kernel:ParticlesTemp);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_TAMD_PARTICLE_H */
