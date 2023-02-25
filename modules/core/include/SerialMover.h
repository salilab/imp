/**
 *  \file IMP/core/SerialMover.h
 *  \brief  A mover that applies other movers one at a time
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SERIAL_MOVER_H
#define IMPCORE_SERIAL_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarlo.h"
#include "MonteCarloMover.h"
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Applies a list of movers one at a time
/** Each time a move is requested, only the next mover in the list
    is applied. This should probably be used in conjunction with
    incremental scoring (MonteCarlo::set_use_incremental()).
 */
class IMPCOREEXPORT SerialMover : public MonteCarloMover {
  int imov_;
  MonteCarloMovers movers_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<MonteCarloMover>(this),
       imov_, movers_);
  }

 public:
  /** Constructor.
      \param[in] mvs list of movers to apply one after another
   */
  SerialMover(const MonteCarloMoversTemp& mvs);

  SerialMover() {}

  const MonteCarloMovers& get_movers() const { return movers_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const override;
  virtual MonteCarloMoverResult do_propose() override;
  virtual void do_reject() override;
  virtual void do_accept() override;
  IMP_OBJECT_METHODS(SerialMover);
};

IMPCORE_END_NAMESPACE

CEREAL_REGISTER_TYPE(IMP::core::SerialMover);

#endif /* IMPCORE_SERIAL_MOVER_H */
