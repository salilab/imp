/**
 *  \file IMP/core/SubsetMover.h
 *  \brief  A mover that applies a random subset of movers
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_SUBSET_MOVER_H
#define IMPCORE_SUBSET_MOVER_H

#include <IMP/core/core_config.h>
#include "MonteCarlo.h"
#include "MonteCarloMover.h"

IMPCORE_BEGIN_NAMESPACE

//! Applies a subset of a list of movers
/** Each time a move is requested, a random fixed-length subset
    of the list of moves is applied. Movers are sampled without
    replacement with uniform probabilities.
 */
class IMPCOREEXPORT SubsetMover : public MonteCarloMover {
  MonteCarloMovers movers_;
  unsigned int n_;
  std::vector<unsigned int> subset_inds_;

 public:
  /** Constructor.
      \param[in] mvs list of movers
      \param[in] n number of movers to be randomly chosen for subset
                   of moves
   */
  SubsetMover(const MonteCarloMoversTemp& mvs, unsigned int n);

  const MonteCarloMovers& get_movers() const { return movers_; }

  unsigned int get_subset_size() const { return n_; }

 protected:
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  virtual void do_accept() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(SubsetMover);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_SUBSET_MOVER_H */
