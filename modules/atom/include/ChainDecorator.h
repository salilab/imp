/**
 *  \file ChainDecorator.h
 *  \brief Store the chain ID
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_CHAIN_DECORATOR_H
#define IMPATOM_CHAIN_DECORATOR_H

#include "config.h"
#include "MolecularHierarchyDecorator.h"
#include <IMP/macros.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! Store info for a chain of a protein
/** \see MolecularHierarchyDecorator
 */
class IMPATOMEXPORT ChainDecorator: public MolecularHierarchyDecorator
{
  IMP_DECORATOR(ChainDecorator, MolecularHierarchyDecorator);
public:
  static ChainDecorator create(Particle *p, char id) {
    p->add_attribute(get_id_key(), id);
    if (!MolecularHierarchyDecorator::is_instance_of(p)) {
      MolecularHierarchyDecorator::create(p,
                     MolecularHierarchyDecorator::CHAIN);
    }
    return ChainDecorator(p);
  }

  static ChainDecorator create(Particle *p, ChainDecorator o) {
    p->add_attribute(get_id_key(), o.get_id());
    if (!MolecularHierarchyDecorator::is_instance_of(p)) {
      MolecularHierarchyDecorator::create(p,
                     MolecularHierarchyDecorator::CHAIN);
    }
    return ChainDecorator(p);
  }


  static bool is_instance_of(Particle *p) {
    return p->has_attribute(get_id_key())
      &&  MolecularHierarchyDecorator::is_instance_of(p);
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


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHAIN_DECORATOR_H */
