/**
 *  \file IMP/pmi/Resolution.h
 *  \brief A decorator for particles with resolution
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPPMI_RESOLUTION_H
#define IMPPMI_RESOLUTION_H

#include <IMP/pmi/pmi_config.h>

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPPMI_BEGIN_NAMESPACE

//! Add resolution to a particle
/** The resolution of the particle is assumed to be in number of residues
    (see \ref pmi_resolution).
 */
class IMPPMIEXPORT Resolution : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi, double resolution) {
    m->add_attribute(get_resolution_key(), pi, resolution);
  }
 public:

  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_resolution_key(), pi);
  }

  Float get_resolution() const {
    return get_model()->get_attribute(get_resolution_key(),
                                      get_particle_index());
  }

  void set_resolution(Float d) { get_model()->set_attribute(get_resolution_key(),
                                                      get_particle_index(),
                                                      d); }

  IMP_DECORATOR_METHODS(Resolution, Decorator);
  /** Add the specified resolution to the particle. */
  IMP_DECORATOR_SETUP_1(Resolution, Float, resolution);

  static FloatKey get_resolution_key();
};

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_RESOLUTION_H */
