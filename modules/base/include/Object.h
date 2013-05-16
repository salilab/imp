/**
 *  \file IMP/base/Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_OBJECT_H
#define IMPBASE_OBJECT_H

#include <IMP/base/base_config.h>
#include "declare_Object.h"
#include "object_cast.h"
#include <sstream>

IMPBASE_BEGIN_NAMESPACE
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
/** Send the whole show output to a stream*/
class ShowFull {
  std::string showed_;

 public:
  ShowFull(Object *o) {
    std::ostringstream oss;
    o->show(oss);
    showed_ = oss.str();
  }
  const std::string &get_string() const { return showed_; }
};
inline std::ostream &operator<<(std::ostream &o, const ShowFull &sf) {
  o << sf.get_string();
  return o;
}
#endif
IMPBASE_END_NAMESPACE

#endif /* IMPBASE_OBJECT_H */
