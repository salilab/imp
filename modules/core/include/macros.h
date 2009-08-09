/**
 *  \file core/macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_MACROS_H
#define IMPCORE_MACROS_H

/** Helper macro for implementing IMP::core::Mover. In
    addition to the IMP_OBJECT methods, it declares
    - Mover::propose_move()
    - Mover::accept_move()
    - Mover::reject_move()
 */
#define IMP_MOVER(Name, version_info)                   \
  virtual void propose_move(Float size);                \
  virtual void accept_move();                           \
  virtual void reject_move();                           \
  IMP_OBJECT(Name, version_info)

#endif  /* IMPCORE_MACROS_H */
