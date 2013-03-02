/**
 *  \file IMP/atom/Chain.h
 *  \brief Store the chain ID
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_CHAIN_H
#define IMPATOM_CHAIN_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/macros.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! Store info for a chain of a protein
/** \see Hierarchy
 */
class IMPATOMEXPORT Chain: public Hierarchy
{
  IMP_DECORATOR(Chain, Hierarchy);
public:
  static Chain setup_particle(Model *m, ParticleIndex pi, char id) {
    m->add_attribute(get_id_key(), pi, id);
    if (!Hierarchy::particle_is_instance(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
    return Chain(m, pi);
  }
  static Chain setup_particle(Particle *p, char id) {
    return setup_particle(p->get_model(),
                          p->get_index(), id);
  }

  static Chain setup_particle(Particle *p, Chain o) {
    p->add_attribute(get_id_key(), o.get_id());
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    return Chain(p);
  }


  static bool particle_is_instance(Particle *p) {
    return p->has_attribute(get_id_key())
      &&  Hierarchy::particle_is_instance(p);
  }

  //! Return the chain id
  char get_id() const {
    return static_cast<char>(get_particle()->get_value(get_id_key()));
  }

  //! Set the chain id
  void set_id(char c) {
    get_particle()->set_value(get_id_key(), c);
  }

  //! The key used to store the chain
  static IntKey get_id_key();
};

IMP_DECORATORS(Chain,Chains, Hierarchies);

/** Get the containing chain or Chain() if there is none*/
IMPATOMEXPORT
Chain get_chain(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHAIN_H */
