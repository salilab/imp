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
 private:
  /**
     Setup a TAMD particle with simulation temperature multiplied by a factor
     tsf, and friction coefficient multiplied by a factor fsf
  */
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                kernel::ParticleIndex pi_ref,
                                float tsf=1.0,
                                float fsf=1.0);

  // just an adaptor for the particle index variety of p_ref
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                kernel::Particle* p_ref,
                                float tsf=1.0,
                                float fsf=1.0) {
    IMP_USAGE_CHECK(m == p_ref->get_model(),
                    "reference particle must be of same model");
    do_setup_particle(m, pi, p_ref->get_index(), tsf, fsf);
  }


 public:
  IMP_DECORATOR_METHODS(TAMDParticle, IMP::Decorator);

  /**
     setup a TAMD variable associated with pi_ref, with temperature scaled by tsf
     and friction scaled by fsf.

     @note Particle is assumed to be decorated as diffusive, XYZ particle.

     @param pi_ref - reference particle index in same model with which
                     this tamd particle is associate
     @param tsf - temperature scale factor
     @param fsf - friction scale factor
   */
  IMP_DECORATOR_SETUP_3(TAMDParticle,
                        kernel::ParticleIndex, pi_ref,
                        Float, tsf,
                        Float, fsf);

  /**
     setup a TAMD variable associated with pi_ref, with temperature scaled by tsf
     and friction scaled by fsf

     @note Particle is assumed to be decorated as diffusive, XYZ particle.

     @param p_ref - reference particle with which this tamd particle is associate
     @param tsf - temperature scale factor
     @param fsf - friction scale factor
   */
  IMP_DECORATOR_SETUP_3(TAMDParticle,
                        kernel::Particle*, p_ref,
                        Float, tsf,
                        Float, fsf);



  //! Return true if the particle is an instance of an TAMDParticle
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex p) {
    return
      m->get_has_attribute(get_reference_particle_index_key(), p) &&
      m->get_has_attribute(get_temperature_scale_factor_key(), p) &&
      m->get_has_attribute(get_friction_scale_factor_key(), p);
  }

  /** returns the particle associated with this TAMD variable
      (eg, centroid of some particles)
  */
  ParticleIndex get_reference_particle_index() const {
    return get_model()->get_attribute(get_reference_particle_index_key(),
                                      get_particle_index());
  }

  //! set temperature factoring for particle relative to simulation
  //! temperature
  void set_temperature_scale_factor(float tsf) {
    get_particle()->set_value(get_temperature_scale_factor_key(), tsf);
  }

  //! get temperature factoring for particle relative to simulation
  //! temperature
  double get_temperature_scale_factor() const {
    return get_model()->get_attribute(get_temperature_scale_factor_key(),
                                      get_particle_index());
  }

  //! set friction factoring for particle relative to particle
  //! diffusion coefficient / rotational diffusion coefficient
  void set_friction_scale_factor(float tsf) {
    get_particle()->set_value(get_friction_scale_factor_key(), tsf);
  }

  //! get friction factoring for particle relative to particle
  //! diffusion coefficient / rotational diffusion coefficient
  double get_friction_scale_factor() const {
    return get_model()->get_attribute(get_friction_scale_factor_key(),
                                      get_particle_index());
  }

  /**
     copy x,y,z coordinates from reference particle.

     @note Assumes both this particle and the reference particle
     are decorated by XYZ (no runtime check is made in release
     mode!)

  */
  void update_coordinates_from_ref();

  //! Get the temperature scale factor key
  static FloatKey get_temperature_scale_factor_key();

  //! Get the temperature scale factor key
  static FloatKey get_friction_scale_factor_key();

  //! Get the temperature scale factor key
  static ParticleIndexKey get_reference_particle_index_key();


};

IMP_DECORATORS(TAMDParticle, TAMDParticles, kernel:ParticlesTemp);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_TAMD_PARTICLE_H */
