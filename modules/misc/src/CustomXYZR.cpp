/**
 *  \file XYZR.cpp   \brief Simple xyzr decorator.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/misc/CustomXYZR.h"

IMPMISC_BEGIN_NAMESPACE

void CustomXYZR::show(std::ostream &out) const {
  out << "(" << get_x() << ", " << get_y() << ", " << get_z() << ": "
      << get_radius() << ")";
}

IMPMISC_END_NAMESPACE
