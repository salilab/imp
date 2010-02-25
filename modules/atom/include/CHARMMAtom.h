/**
 *  \file atom/CHARMMAtom.h
 *  \brief A decorator for an atom that has a defined CHARMM type.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_ATOM_H
#define IMPATOM_CHARMM_ATOM_H

#include "config.h"

#include "Atom.h"

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for an atom that has a defined CHARMM type.
class IMPATOMEXPORT CHARMMAtom : public Atom
{
public:
  IMP_DECORATOR(CHARMMAtom, Atom)

  /** Create a decorator with the passed CHARMM type.
      The particle is assumed to already have all atom attributes.
   */
  static CHARMMAtom setup_particle(Particle *p, String charmm_type) {
    IMP_USAGE_CHECK(Atom::particle_is_instance(p),
                    "Particle must already be an Atom particle");
    p->add_attribute(get_charmm_type_key(), charmm_type);
    return CHARMMAtom(p);
  }

  IMP_DECORATOR_GET_SET(charmm_type, get_charmm_type_key(), String, String);

  //! Return true if the particle is an instance of a CHARMMAtom
  static bool particle_is_instance(Particle *p) {
    return Atom::particle_is_instance(p)
           && p->has_attribute(get_charmm_type_key());
  }

  static StringKey get_charmm_type_key();
};

IMP_OUTPUT_OPERATOR(CHARMMAtom);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHARMM_ATOM_H */
