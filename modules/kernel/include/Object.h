/**
 *  \file IMP/kernel/Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OBJECT_H
#define IMPKERNEL_OBJECT_H

#include <IMP/kernel/kernel_config.h>
#include <IMP/base/Object.h>
#include <IMP/base/types.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/WeakPointer.h>
#include <IMP/base/SetLogState.h>
#include <IMP/base/SetCheckState.h>

IMPKERNEL_BEGIN_NAMESPACE
#ifndef SWIG
using base::Object;
using base::Objects;
using base::ObjectsTemp;
#endif
IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_OBJECT_H */
