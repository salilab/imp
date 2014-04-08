/**
 *  \file Domain.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/Domain.h"

IMPATOM_BEGIN_NAMESPACE

const Domain::Data& Domain::get_data() {
  static Data data;
  return data;
}

void Domain::show(std::ostream& out) const {
  IntRange range = get_index_range();
  out << "Domain: " << range.first << " to " << range.second;
}

IMPATOM_END_NAMESPACE
