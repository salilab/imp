/**
 *  \file MoveStatisticsScoreState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPSTATISTICS_MOVE_STATISTICS_SCORE_STATE_H
#define IMPSTATISTICS_MOVE_STATISTICS_SCORE_STATE_H

#include "statistics_config.h"
#include <IMP/OptimizerState.h>
#include <IMP/internal/utility.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/io.h>

IMPSTATISTICS_BEGIN_NAMESPACE

//! Keep track of statistics about how particles move.
/** Keep track of average and maximum moves for a set
    of particles during optimization.
 */
class IMPSTATISTICSEXPORT MoveStatisticsScoreState: public ScoreState
{
  Particles ps_;
  algebra::Vector3Ds last_;
  double max_move_;
  std::string max_mover_;
  double max_average_;
  double total_move_;
  double total_movers_;
  bool init_;
public:
  MoveStatisticsScoreState(const ParticlesTemp& ps);
  void show_statistics(std::ostream &out=std::cout) const;
  void reset();
  IMP_SCORE_STATE(MoveStatisticsScoreState);
};


IMPSTATISTICS_END_NAMESPACE

#endif  /* IMPSTATISTICS_MOVE_STATISTICS_SCORE_STATE_H */
