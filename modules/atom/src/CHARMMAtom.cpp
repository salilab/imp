/**
 *  \file atom/CHARMMAtom.h
 *  \brief A decorator for an atom that has a defined CHARMM type.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/CHARMMAtom.h>
#include <IMP/atom/Hierarchy.h>

IMPATOM_BEGIN_NAMESPACE

StringKey CHARMMAtom::get_charmm_type_key() {
  static StringKey k("CHARMM atom type");
  return k;
}

void CHARMMAtom::show(std::ostream &out) const {
  Atom::show(out);
  out << " CHARMM type= " << get_charmm_type();
}

namespace {
struct FindUntypedVisitor {
  Atoms *atoms_;
  FindUntypedVisitor(Atoms *atoms) : atoms_(atoms) {}
  bool operator()(Hierarchy h) {
    if (h.get_as_atom() && !CHARMMAtom::get_is_setup(h)) {
      atoms_->push_back(Atom(h));
    }
    return true;
  }
};
}

Atoms get_charmm_untyped_atoms(Hierarchy hierarchy) {
  Atoms atoms;
  FindUntypedVisitor visitor(&atoms);
  IMP::core::visit_depth_first(hierarchy, visitor);
  return atoms;
}

void remove_charmm_untyped_atoms(Hierarchy hierarchy) {
  Atoms untyped = get_charmm_untyped_atoms(hierarchy);
  for (Atoms::iterator it = untyped.begin(); it != untyped.end(); ++it) {
    destroy(Hierarchy(it->get_particle()));
  }
}

IMPATOM_END_NAMESPACE
