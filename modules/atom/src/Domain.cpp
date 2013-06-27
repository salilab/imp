/**
 *  \file Domain.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Domain.h"

IMPATOM_BEGIN_NAMESPACE

const Domain::Data& Domain::get_data() {
  static Data data;
  return data;
}

void Domain::show(std::ostream& out) const {
  out << "Domain: " << get_begin_index() << " to " << get_end_index();
}

Int Domain::get_begin_index() const {
    return get_particle()->get_value(get_data().begin);
  }

Int Domain::get_end_index() const {
  return get_particle()->get_value(get_data().end);
}


IMPATOM_END_NAMESPACE
