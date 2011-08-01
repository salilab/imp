/**
 *  \file SerialMover.h
 *  \brief  A mover that apply other movers one at a time
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SERIAL_MOVER_H
#define IMPCORE_SERIAL_MOVER_H

#include "core_config.h"
#include "MonteCarlo.h"
#include "Mover.h"
#include "core_macros.h"

IMPCORE_BEGIN_NAMESPACE

//! Apply a list of movers one at a time
/** Each time a move is requested, on the next mover is applied.
 */
class IMPCOREEXPORT SerialMover : public Mover
{
public:
  /** The Serial are applied one at a time
      \param[in] mvs list of Serial
   */
  SerialMover(const MoversTemp& mvs);
  IMP_MOVER(SerialMover);
private:
  core::Movers mvs_;
  int imov_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SERIAL_MOVER_H */
