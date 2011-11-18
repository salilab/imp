/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/NodeTupleHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/RootHandle.h>

namespace RMF {

std::string get_tuple_type_name(NodeTupleType t) {
  switch (t) {
  case BOND:
    return "bond";
  case CUSTOM_TUPLE:
    return "custom";
  default:
    return "unknown";
  }
}
} /* namespace RMF */
