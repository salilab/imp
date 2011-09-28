/**
 *  \file IMP/base_types.h    \brief Basic types used by IMP.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_BASE_TYPES_H
#define IMPKERNEL_BASE_TYPES_H

#include "kernel_config.h"
#include "Key.h"
#include "macros.h"
#include <IMP/base/types.h>

IMP_BEGIN_NAMESPACE


/** @name Attribute Keys
    Each type of attribute has an associated type of key. The keys can
    be constructed from a string. Such construction can be expensive
    and so the resulting keys should be cached.
    @{
 */

//! The type used to identify float attributes in the Particles
IMP_DECLARE_KEY_TYPE(FloatKey, 0);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntKey, 1);
//! The type used to identify string attributes in the Particles
IMP_DECLARE_KEY_TYPE(StringKey, 2);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticleKey, 3);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectKey, 4);
//! The type used to identify int attributes in the Particles
IMP_DECLARE_KEY_TYPE(IntsKey, 5);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ParticlesKey, 6);
//! The type used to identify a particle attribute in the Particles
IMP_DECLARE_KEY_TYPE(ObjectsKey, 7);

/** @} */

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
inline void show(std::ostream &out, int i) {
  out << i;
}

inline void show(std::ostream &out, unsigned int i) {
  out << i;
}

inline void show(std::ostream &out, std::string i) {
  out << i;
}

inline void show(std::ostream &out, double i) {
  out << i;
}
template <class T>
inline void show(std::ostream &out, const T& i) {
  out << i;
}
template <class T>
inline void show(std::ostream &out, const T*i) {
  out << i->get_name();
}
template <class T>
inline void show(std::ostream &out, T*i) {
  out << i->get_name();
}
#endif

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_BASE_TYPES_H */
