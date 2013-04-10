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
#include "MonteCarloMover.h"

IMPCORE_BEGIN_NAMESPACE

//! Apply a list of movers one at a time
/** Each time a move is requested, on the next mover is applied. This
    should probably be used in conjunction with incremental scoring
    (MonteCarlo::set_use_incremental()).
 */
class IMPCOREEXPORT SerialMover : public MonteCarloMover
{
  int imov_;
  MonteCarloMovers movers_;
public:
  /** The Serial are applied one at a time
      \param[in] mvs list of movers to apply one after another
   */
  SerialMover(const MonteCarloMoversTemp& mvs);

  const MonteCarloMovers& get_movers() const {
    return movers_;
  }

#ifndef IMP_DOXYGEN
  IMP_DEPRECATED_WARN
  double get_acceptance_probability(int i) const {
    return static_cast<double>(movers_[i]->get_number_of_accepted())/
      movers_[i]->get_number_of_proposed();
  }
  IMP_DEPRECATED_WARN
  void reset_acceptance_probabilities() {
    for (unsigned int i = 0; i < movers_.size(); ++i) {
      movers_[i]->reset_statistics();
    }
  }
  IMP_DEPRECATED_WARN
    unsigned int get_number_of_movers() const {
    return movers_.size();
  }
#endif
protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  virtual void do_accept() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SerialMover);
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SERIAL_MOVER_H */
