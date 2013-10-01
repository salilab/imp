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
class IMPATOMEXPORT Chain : public Hierarchy {

  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                char id) {
    m->add_attribute(get_id_key(), pi, id);
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                Chain o) {
    do_setup_particle(m, pi, o.get_id());
  }

 public:
  IMP_DECORATOR_METHODS(Chain, Hierarchy);
  IMP_DECORATOR_SETUP_1(Chain, char, id);
  IMP_DECORATOR_SETUP_1(Chain, Chain, other);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_id_key(), pi) &&
           Hierarchy::get_is_setup(m, pi);
  }

  //! Return the chain id
  char get_id() const {
    return static_cast<char>(
        get_model()->get_attribute(get_id_key(), get_particle_index()));
  }

  //! Set the chain id
  void set_id(char c) {
    get_model()->set_attribute(get_id_key(), get_particle_index(), c);
  }

  //! The key used to store the chain
  static IntKey get_id_key();
};

IMP_DECORATORS(Chain, Chains, Hierarchies);

/** Get the containing chain or Chain() if there is none*/
IMPATOMEXPORT Chain get_chain(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHAIN_H */
