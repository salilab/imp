/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/NodeSetHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/FileHandle.h>

namespace RMF {

std::string get_set_type_name(NodeSetType t) {
  switch (t) {
  case BOND:
    return "bond";
  case CUSTOM_SET:
    return "custom";
  default:
    return "unknown";
  }
}
} /* namespace RMF */
