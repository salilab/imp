/**
 *  \file IMP/core/NeighborsTable.h
 *  \brief Angle restraint between three particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_NEIGHBORS_TABLE_H
#define IMPCORE_NEIGHBORS_TABLE_H

#include <IMP/core/core_config.h>
#include "internal/CoreClosePairContainer.h"
#include <IMP/kernel/SingletonContainer.h>
#include <IMP/kernel/ScoreState.h>
#include <IMP/base/Pointer.h>
#include <boost/unordered_map.hpp>

IMPCORE_BEGIN_NAMESPACE

/** Maintain a table that can be used to look up the neighbors
    of particles. That is, you can efficiently find all
    particles that are within a certain distance of a passed one.

    As with the container::ClosePairContainer, there may be some
    neighbors returned that are not close neighbors, but all close
    neighbors will be returned.
*/
class IMPCOREEXPORT NeighborsTable : public kernel::ScoreState {
  base::PointerMember<kernel::PairContainer> input_;
  std::size_t input_version_;
  boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes> data_;

 protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return kernel::ModelObjectsTemp(1, input_);
  }
  virtual kernel::ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE {
    return kernel::ModelObjectsTemp();
  }
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *) IMP_OVERRIDE {}

 public:
  NeighborsTable(kernel::PairContainer *input,
                 std::string name = "CloseNeighborsTable%1%");
  /** Return all kernel::ParticleIndexes that are within the distance threshold
      of this one (plus some that are aren't, for efficiency). */
  const kernel::ParticleIndexes &get_neighbors(kernel::ParticleIndex pi) const {
    set_was_used(true);
    return data_.find(pi)->second;
  }
  IMP_OBJECT_METHODS(NeighborsTable);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_NEIGHBORS_TABLE_H */
