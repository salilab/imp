/**
 *  \file IMP/kernel/Particle.h
 *  \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_PARTICLE_H
#define IMPKERNEL_PARTICLE_H

#include <IMP/kernel/kernel_config.h>
#include "declare_Particle.h"
#include "Model.h"
#include "Decorator.h"

IMPKERNEL_BEGIN_NAMESPACE

// for swig
class Decorator;
class Particle;

/** Take Decorator or Particle. */
class ParticleAdaptor: public base::InputAdaptor {
  Model *m_;
  ParticleIndex pi_;
 public:
  ParticleAdaptor(): m_(nullptr), pi_() {}
  ParticleAdaptor(Particle *p): m_(p->get_model()),
    pi_(p->get_index()) {}
  ParticleAdaptor(const Decorator& d) : m_(d.get_model()),
    pi_(d.get_particle_index()){}
#ifndef SWIG
  ParticleAdaptor(IMP::base::Pointer<Particle> p): m_(p->get_model()),
                                                   pi_(p->get_index()) {}
  ParticleAdaptor(IMP::base::WeakPointer<Particle> p): m_(p->get_model()),
                                                   pi_(p->get_index()) {}
  ParticleAdaptor(IMP::base::OwnerPointer<Particle> p): m_(p->get_model()),
                                                   pi_(p->get_index()) {}
#endif
  Model *get_model() const {return m_;}
  ParticleIndex get_particle_index() const {return pi_;}
};


IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_PARTICLE_H */
