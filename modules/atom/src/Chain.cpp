/**
 *  \file Chain.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Chain.h"

IMPATOM_BEGIN_NAMESPACE

StringKey Chain::get_id_key() {
  static StringKey k("chain");
  return k;
}

void Chain::show(std::ostream &out) const { out << "Chain " << get_id(); }

Chain get_chain(Hierarchy h) {
  do {
    if (h == Hierarchy()) {
      return Chain();
    }

    if (Chain::get_is_setup(h)) {
      return Chain(h);
    }
  } while ((h = h.get_parent()));
  return Chain();
}

IMPATOM_END_NAMESPACE
