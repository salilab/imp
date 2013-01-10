/**
 *  \file IMP/core/SerialMover.h
 *  \brief  A mover that apply other movers one at a time
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SERIAL_MOVER_H
#define IMPCORE_SERIAL_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarlo.h"
#include "Mover.h"
#include "mover_macros.h"

IMPCORE_BEGIN_NAMESPACE

//! Apply a list of movers one at a time
/** Each time a move is requested, on the next mover is applied. This
    should probably be used in conjunction with incremental scoring
    (MonteCarlo::set_use_incremental()).
 */
class IMPCOREEXPORT SerialMover : public Mover
{
public:
  /** The Serial are applied one at a time
      \param[in] mvs list of movers to apply one after another
   */
  SerialMover(const MoversTemp& mvs);

  IMP_LIST_ACTION(public, Mover, Movers, mover, movers, Mover*, Movers,
                  {
                    obj->set_optimizer(get_optimizer());
                    obj->set_was_used(true);
                    reset_acceptance_probabilities();
                  },{},{
                    obj->set_optimizer(nullptr);
                    if (container) {
                      container->reset_acceptance_probabilities();
                    }
                  });
  /** Get the acceptance rate for mover i. This is reset when
      reset is called or the set of movers is changed.*/
  double get_acceptance_probability(int i) const;
  void   reset_acceptance_probabilities();

  IMP_MOVER(SerialMover);
private:
  int imov_;
  Floats failed_;
  Floats attempt_;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SERIAL_MOVER_H */
