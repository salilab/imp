/**
 *  \file OwnerPointer.h
 *  \brief A NULL-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_OWNER_POINTER_H
#define IMPKERNEL_INTERNAL_OWNER_POINTER_H

#include "../kernel_config.h"
#include <IMP/base/internal/PointerBase.h>

IMP_BEGIN_INTERNAL_NAMESPACE

template <class O>
struct OwnerPointer:
    IMP::base::internal::PointerBase<IMP::base::internal
                                     ::OwnerPointerTraits<O> > {
  template <class Any>
  OwnerPointer(const Any &o):
      IMP::base::internal::PointerBase<IMP::base
                                       ::internal::OwnerPointerTraits<O> >(o){}
  OwnerPointer(){}
};


IMP_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_OWNER_POINTER_H */
