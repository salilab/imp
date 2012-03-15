/**
 *  \file base/Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_OBJECT_H
#define IMPBASE_OBJECT_H

#include "base_config.h"
#include "declare_Object.h"


IMPBASE_BEGIN_NAMESPACE

#if !defined(IMP_DOXYGEN) && !defined(IMP_SWIG)
inline std::ostream &operator<<(std::ostream &out, const Object& o) {
  o.show(out);
  return out;
}
#endif



/** Up (or down) cast an \imp Object-derived class. If the cast
    does not succeed a ValueException will be thrown. Use a
    \c dynamic_cast if you prefer to have a NULL returned.
 */
template <class O, class I>
inline O* object_cast(I *o) {
  O *ret= dynamic_cast<O*>(o);
  if (!ret) {
    if (!o) {
      IMP_THROW("Cannot cast NULL pointer to desired type.", ValueException);
    } else {
      IMP_THROW("Object " << o->get_name() << " cannot be cast to "
                << "desired type.", ValueException);
    }
  }
  return ret;
}


IMPBASE_END_NAMESPACE

#include "SetLogState.h"

#endif  /* IMPBASE_OBJECT_H */
