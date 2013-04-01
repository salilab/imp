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

/** Helper macro for implementing IMP::core::MonteCarlo. In
    addition to the IMP_OBJECT methods, it declares
    - IMP::core::MonteCarlo::do_step()
 */
#define IMP_MONTE_CARLO(Name)                           \
  IMP_PRAGMA(warn do not use this macro)                \
  IMP_IMPLEMENT(virtual void do_step());                \
  IMP_OBJECT(Name)


#endif  /* IMPCORE_MONTE_CARLO_MACROS_H */
