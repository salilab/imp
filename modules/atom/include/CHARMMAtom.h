/**
 *  \file IMP/atom/CHARMMAtom.h
 *  \brief A decorator for an atom that has a defined CHARMM type.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARMM_ATOM_H
#define IMPATOM_CHARMM_ATOM_H

#include <IMP/atom/atom_config.h>

#include "Atom.h"
#include "Hierarchy.h"
#include <IMP/kernel/decorator_macros.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for an atom that has a defined CHARMM type.
class IMPATOMEXPORT CHARMMAtom : public Atom
{
public:
  IMP_DECORATOR(CHARMMAtom, Atom);

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

//! Get all atoms in the Hierarchy that do not have CHARMM types.
/** \return a list of every Atom in the given Hierarchy that is not also
            a CHARMMAtom.
    \see remove_charmm_untyped_atoms
 */
IMPATOMEXPORT Atoms get_charmm_untyped_atoms(Hierarchy hierarchy);

//! Remove any atom from the Hierarchy that does not have a CHARMM type.
/** \see get_charmm_untyped_atoms, CHARMMTopology::add_missing_atoms
 */
IMPATOMEXPORT void remove_charmm_untyped_atoms(Hierarchy hierarchy);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_CHARMM_ATOM_H */
