/**
 *  \file OwnerPointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_INTERNAL_OWNER_POINTER_H
#define IMP_INTERNAL_OWNER_POINTER_H


#include "PointerBase.h"

IMP_BEGIN_INTERNAL_NAMESPACE

template <class O>
struct OwnerPointer: internal::PointerBase<O, internal::OwnerPointerTraits> {
  template <class Any>
  OwnerPointer(const Any &o):
    internal::PointerBase<O, internal::OwnerPointerTraits>(o){}
  OwnerPointer(){}
};


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMP_INTERNAL_OWNER_POINTER_H */
