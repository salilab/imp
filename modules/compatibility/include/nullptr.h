/**
 *  \file compatibility/nullptr.h
 *  \brief Provide a nullptr keyword analog.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPCOMPATIBILITY_NULLPTR_H
#define IMPCOMPATIBILITY_NULLPTR_H

#include "compatibility_config.h"

namespace IMP {
#ifdef IMP_DOXYGEN
/** The C++0x standard adds the nullptr keyword to get around a variety of
    problems with NULL. We provide an emulation within the IMP namespace when
    it is not available.
*/
const std::nullptr_t nullptr;

#else

#if IMP_DEFINE_NULLPTR

#if !defined(SWIG)
struct nullptr_t {
  template <class O>
  operator O*() const {
    return static_cast<O*>(NULL);
  }
  /*template <class O, class C>
  operator O C::*() const {
    return static_cast<const O*>(NULL);
    }*/
};
template <class O>
inline bool operator==(O *a, nullptr_t o) {
  return a == static_cast<O*>(o);
}
template <class O>
inline bool operator!=(O *a, nullptr_t o) {
  return a != static_cast<O*>(o);
}
extern IMPCOMPATIBILITYEXPORT const nullptr_t nullptr;
#else
extern const void * const nullptr;
#endif //SWIG

#endif // IMP_DEFINE_NULLPTR
#endif // IMP_DOXYGEN

}

#endif  /* IMPCOMPATIBILITY_NULLPTR_H */
