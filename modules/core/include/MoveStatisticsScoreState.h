/**
 *  \file IMP/core/MoveStatisticsScoreState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_MOVE_STATISTICS_SCORE_STATE_H
#define IMPCORE_MOVE_STATISTICS_SCORE_STATE_H

#include <IMP/core/core_config.h>
#include <IMP/OptimizerState.h>
#include <IMP/kernel/internal/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/score_state_macros.h>
#include <IMP/io.h>

IMPCORE_BEGIN_NAMESPACE

//! Keep track of statistics about how particles move.
/** Keep track of average and maximum moves for a set
    of particles during optimization.
 */
class IMPCOREEXPORT MoveStatisticsScoreState : public ScoreState {
  kernel::Particles ps_;
  algebra::Vector3Ds last_;
  double max_move_;
  std::string max_mover_;
  double max_average_;
  double total_move_;
  double total_movers_;
  bool init_;

 public:
  MoveStatisticsScoreState(const kernel::ParticlesTemp& ps);
  void show_statistics(std::ostream& out = std::cout) const;
  void reset();
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator* da) IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const;
  virtual kernel::ModelObjectsTemp do_get_outputs() const;
  IMP_OBJECT_METHODS(MoveStatisticsScoreState);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MOVE_STATISTICS_SCORE_STATE_H */
