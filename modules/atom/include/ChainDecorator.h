/**
 *  \file ChainDecorator.h
 *  \brief Store the chain ID
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_CHAIN_DECORATOR_H
#define IMPATOM_CHAIN_DECORATOR_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/macros.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! Store info for a chain of a protein
/** \see MolecularHierarchyDecorator
 */
class IMPATOMEXPORT ChainDecorator: public Decorator
{
  IMP_DECORATOR(ChainDecorator, Decorator);
public:
  static ChainDecorator create(Particle *p, char id) {
    p->add_attribute(get_id_key(), id);
    return ChainDecorator(p);
  }

  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_id_key());
  }
  //! Return the chain id
  char get_chain() const {
    return get_particle()->get_value(get_id_key());
  }
  //! Set the chain id
  void set_chain(char c) {
    get_particle()->set_value(get_id_key(), c);
  }

  //! The key used to store the chain
  static IntKey get_id_key();
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHAIN_DECORATOR_H */
