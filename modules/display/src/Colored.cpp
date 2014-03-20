/**
 *  \file Colored.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/Colored.h"

IMPDISPLAY_BEGIN_NAMESPACE

void Colored::show(std::ostream& out) const {
  out << "Colored " << get_color() << std::endl;
}

namespace {
FloatKeys init_color_keys() {
  FloatKeys ret;
  ret.push_back(FloatKey("display red"));
  ret.push_back(FloatKey("display green"));
  ret.push_back(FloatKey("display blue"));
  return ret;
}
}

const FloatKeys& Colored::get_color_keys() {
  static FloatKeys rks = init_color_keys();
  return rks;
}

IMPDISPLAY_END_NAMESPACE
