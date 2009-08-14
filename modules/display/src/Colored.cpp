/**
 *  \file Colored.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/Colored.h"
#include <boost/assign/list_of.hpp>


IMPDISPLAY_BEGIN_NAMESPACE


void Colored::show(std::ostream &out) const {
  out << "Colored " << get_color() << std::endl;
}

FloatKeys Colored::get_color_keys() {
  static FloatKeys rks=boost::assign::list_of(FloatKey("display red"))
    (FloatKey("display green"))
    (FloatKey("display blue"));
  return rks;
}

IMPDISPLAY_END_NAMESPACE
