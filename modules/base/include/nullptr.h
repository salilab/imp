/**
 *  \file IMP/base/nullptr.h
 *  \brief Provide a nullptr keyword analog.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_NULLPTR_H
#define IMPBASE_NULLPTR_H

#include <IMP/base/base_config.h>

namespace IMP {
#ifdef IMP_DOXYGEN
/** The C++0x standard adds the nullptr keyword to get around a variety of
    problems with NULL. We provide an emulation within the IMP namespace when
    it is not available.

    Use "nullptr" in code; the compiler will use our IMP::nullptr emulation
    automatically on systems that don't provide a native nullptr implementation.

    If you are not in the IMP namespace, use the IMP_NULLPTR macro rather
    than asking for "IMP::nullptr". The latter does not work with some compilers
    (e.g. MSVC, which gets confused because nullptr is a keyword).
*/
const std::nullptr_t nullptr;

#elif !IMP_COMPILER_HAS_NULLPTR

#if !defined(SWIG)

#if __GNUC__ && __GNUC__==4 && __GNUC_MINOR__>=6
IMP_GCC_PRAGMA(diagnostic push)
IMP_GCC_PRAGMA(diagnostic ignored "-Wc++0x-compat")
#endif


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
extern IMPBASEEXPORT const nullptr_t nullptr;

#if __GNUC__ && __GNUC__==4 && __GNUC_MINOR__>=6
IMP_GCC_PRAGMA(diagnostic pop)
#endif

#else // SWIG
extern const void * const nullptr;
#endif //SWIG
#endif // IMP_DOXYGEN

}

#endif  /* IMPBASE_NULLPTR_H */
