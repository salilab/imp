/**
 *  \file atom/CHARMMAtom.h
 *  \brief A decorator for an atom that has a defined CHARMM type.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/CHARMMAtom.h>

IMPATOM_BEGIN_NAMESPACE

StringKey CHARMMAtom::get_charmm_type_key() {
  static StringKey k("CHARMM atom type");
  return k;
}

void CHARMMAtom::show(std::ostream &out) const
{
  Atom::show(out);
  out << " CHARMM type= " << get_charmm_type();
}

IMPATOM_END_NAMESPACE
