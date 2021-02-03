/**
 *  \file IMP/spb/HelixDecorator.h
 *  \brief Add helix parameters to a particle.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_HELIX_DECORATOR_H
#define IMPSPB_HELIX_DECORATOR_H

#include "spb_config.h"

#include <IMP/Decorator.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/decorator_macros.h>

IMPSPB_BEGIN_NAMESPACE

//! Add helix parameters to a particle.
class IMPSPBEXPORT HelixDecorator : public Decorator {
  static FloatKey get_helix_key(unsigned int i);

 public:
  IMP_DECORATOR_METHODS(HelixDecorator, Decorator);

  static HelixDecorator setup_particle(Particle *p, Float b, Float e) {
    p->add_attribute(get_helix_key(0), b);
    p->add_attribute(get_helix_key(1), e);
    HelixDecorator boundaries(p->get_model(), p->get_index());
    return boundaries;
  }

  //! Get the beginning
  Float get_begin() const {
    return get_particle()->get_value(get_helix_key(0));
  }

  //! Get the end
  Float get_end() const { return get_particle()->get_value(get_helix_key(1)); }

  static bool get_is_setup(IMP::Model *m, IMP::ParticleIndex id) {
    IMP_USAGE_CHECK(
        (m->get_particle(id)->has_attribute(get_helix_key(0)) &&
         m->get_particle(id)->has_attribute(get_helix_key(1))) ||
            (!m->get_particle(id)->has_attribute(get_helix_key(0)) &&
             !m->get_particle(id)->has_attribute(get_helix_key(1))),
        "Particle expected to either begin and end or none.");
    return m->get_particle(id)->has_attribute(get_helix_key(1));
  }

  //! set begin
  void set_begin(Float v) { get_particle()->set_value(get_helix_key(0), v); }

  //! set end
  void set_end(Float v) { get_particle()->set_value(get_helix_key(1), v); }
};

IMP_VALUES(HelixDecorator, HelixDecorators);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_HELIX_DECORATOR_H */
