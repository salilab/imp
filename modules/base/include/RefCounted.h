/**
 *  \file IMP/base/RefCounted.h
 *  \brief A common base class for ref counted objects.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_REF_COUNTED_H
#define IMPBASE_REF_COUNTED_H

#include <IMP/base/base_config.h>
#include "exception.h"
#include "utility_macros.h"
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include "NonCopyable.h"
#include "Object.h"

IMPBASE_DEPRECATED_HEADER(2.2, "Use Object.h instead.h")

IMPBASE_BEGIN_NAMESPACE
/** \deprecated_at{2.2} Just use Object. */
typedef Object RefCounted;
IMPBASE_END_NAMESPACE

#endif /* IMPBASE_REF_COUNTED_H */
