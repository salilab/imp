/**
 *  \file OwnerPointer.h
 *  \brief A nullptr-initialized pointer to an IMP Object.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_OWNER_POINTER_H
#define IMPKERNEL_INTERNAL_OWNER_POINTER_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/internal/PointerBase.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

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


IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_OWNER_POINTER_H */
