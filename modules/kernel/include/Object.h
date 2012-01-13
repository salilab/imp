/**
 *  \file IMP/Object.h
 *  \brief A shared base class to help in debugging and things.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_OBJECT_H
#define IMPKERNEL_OBJECT_H

#include "kernel_config.h"
#include <IMP/base/Object.h>
#include <IMP/base/Pointer.h>
#include <IMP/base/WeakPointer.h>
#include <IMP/base/SetLogState.h>
#include <IMP/base/SetCheckState.h>

IMP_BEGIN_NAMESPACE
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
using IMP::base::Object;
//using IMP::base::Objects;
using IMP::base::object_cast;
using IMP::base::Objects;
using IMP::base::ObjectsTemp;
#endif

IMP_END_NAMESPACE


#include "VectorOfRefCounted.h"

#endif  /* IMPKERNEL_OBJECT_H */
