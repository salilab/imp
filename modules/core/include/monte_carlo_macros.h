/**
 *  \file IMP/core/monte_carlo_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MONTE_CARLO_MACROS_H
#define IMPCORE_MONTE_CARLO_MACROS_H

#include <IMP/base/doxygen_macros.h>
#include <IMP/base/object_macros.h>

/** You should just declare the needed functions yourself.
 */
#define IMP_MONTE_CARLO(Name)                          \
  IMP_DEPRECATED_MACRO(2.0, "Define do_step yourself") \
      IMP_IMPLEMENT(virtual void do_step());           \
  IMP_OBJECT_NO_WARNING(Name)

#endif /* IMPCORE_MONTE_CARLO_MACROS_H */
