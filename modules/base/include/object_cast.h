/**
 *  \file IMP/base/object_cast.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_OBJECT_CAST_H
#define IMPBASE_OBJECT_CAST_H

#include <IMP/base/base_config.h>
#include "Object.h"
#include "check_macros.h"


IMPBASE_BEGIN_NAMESPACE

/** Up (or down) cast an \imp Object-derived class. If the cast
    does not succeed a ValueException will be thrown. Use a
    \c dynamic_cast if you prefer to have a nullptr returned.
 */
template <class O>
inline O* object_cast(Object *o) {
  O *ret= dynamic_cast<O*>(o);
  if (!ret) {
    if (!o) {
      IMP_THROW("Cannot cast nullptr pointer to desired type.", ValueException);
    } else {
      IMP_THROW("Object " << o->get_name() << " cannot be cast to "
                << "desired type.", ValueException);
    }
  }
  return ret;
}


IMPBASE_END_NAMESPACE


#endif  /* IMPBASE_OBJECT_CAST_H */
