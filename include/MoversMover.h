/**
 *  \file MoversMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_MOVERS_MOVER_H
#define IMPMEMBRANE_MOVERS_MOVER_H

#include "membrane_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/Mover.h>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

//! Apply a list of movers one at a time
/**
 */
class IMPMEMBRANEEXPORT MoversMover : public core::Mover
{
public:
  /** The Movers are applied one at a time
      \param[in] mvs list of Movers
   */
  MoversMover(core::Movers mvs);
  void reset_move();
  void propose_move(Float f);
  IMP_OBJECT(MoversMover);
private:
  core::Movers mvs_;
  int imov_;
};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_MOVERS_MOVER_H */
