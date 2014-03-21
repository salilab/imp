/**
 *  \file IMP/atom/Chain.h
 *  \brief Store the chain ID
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
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
                                std::string id) {
    m->add_attribute(get_id_key(), pi, id);
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                char c) {
    do_setup_particle(m, pi, std::string(1, c));
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                Chain o) {
    do_setup_particle(m, pi, o.get_id());
  }

 public:
  IMP_DECORATOR_METHODS(Chain, Hierarchy);
  IMP_DECORATOR_SETUP_1(Chain, std::string, id);
  IMP_DECORATOR_SETUP_1(Chain, char, id);
  IMP_DECORATOR_SETUP_1(Chain, Chain, other);

  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return m->get_has_attribute(get_id_key(), pi) &&
           Hierarchy::get_is_setup(m, pi);
  }

  //! Return the chain id
  std::string get_id() const {
    return get_model()->get_attribute(get_id_key(), get_particle_index());
  }

  //! Return the chain id
  /** \deprecated_at{2.2} Use the string version. */
  IMPATOM_DEPRECATED_FUNCTION_DECL(2.2)
  char get_id_char() const {
    return get_model()->get_attribute(get_id_key(), get_particle_index())[0];
  }

  //! Set the chain id
  void set_id(std::string c) {
    get_model()->set_attribute(get_id_key(), get_particle_index(), c);
  }

  //! The key used to store the chain
  static StringKey get_id_key();
};

IMP_DECORATORS(Chain, Chains, Hierarchies);

/** Get the containing chain or Chain() if there is none*/
IMPATOMEXPORT Chain get_chain(Hierarchy h);

/** Walk up the hierarchy to determine the chain id. */
IMPATOMEXPORT std::string get_chain_id(Hierarchy h);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHAIN_H */
