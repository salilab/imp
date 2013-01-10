/**
 *  \file IMP/core/mover_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MOVER_MACROS_H
#define IMPCORE_MOVER_MACROS_H

#include <IMP/base/doxygen_macros.h>
#include "Mover.h"

/** Helper macro for implementing IMP::core::Mover. In
    addition to the IMP_OBJECT methods, it declares
    - IMP::core::Mover::propose_move()
    - IMP::core::Mover::reset_move()
 */
#define IMP_MOVER(Name)                                 \
  IMP_IMPLEMENT(virtual ParticlesTemp propose_move(Float size));        \
  IMP_IMPLEMENT(virtual void reset_move());                             \
  IMP_IMPLEMENT(virtual ParticlesTemp get_output_particles() const);    \
  IMP_OBJECT(Name)


#endif  /* IMPCORE_MOVER_MACROS_H */
