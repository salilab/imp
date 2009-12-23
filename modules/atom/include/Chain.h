/**
 *  \file Chain.h
 *  \brief Store the chain ID
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_CHAIN_H
#define IMPATOM_CHAIN_H

#include "config.h"
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
  static Chain setup_particle(Particle *p, char id) {
    p->add_attribute(get_id_key(), id);
    if (!Hierarchy::particle_is_instance(p)) {
      Hierarchy::setup_particle(p);
    }
    return Chain(p);
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
    return get_particle()->get_value(get_id_key());
  }

  //! Set the chain id
  void set_id(char c) {
    get_particle()->set_value(get_id_key(), c);
  }

  //! The key used to store the chain
  static IntKey get_id_key();
};

IMP_DECORATORS(Chain, Hierarchies);

IMP_OUTPUT_OPERATOR(Chain);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHAIN_H */
