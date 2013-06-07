/**
 *  \file Clustering.cpp   \brief Holds data points to cluster using k-means
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core/MoveStatisticsScoreState.h>
#include <IMP/core/XYZ.h>
#include <boost/format.hpp>

IMPCORE_BEGIN_NAMESPACE
MoveStatisticsScoreState::MoveStatisticsScoreState(const ParticlesTemp &ps)
    : ScoreState(ps[0]->get_model(), "MoveStatisticsScoreState%1%"),
      ps_(ps.begin(), ps.end()) {
  reset();
}

void MoveStatisticsScoreState::do_before_evaluate() {
  double cur_total_move = 0;
  for (unsigned int i = 0; i < ps_.size(); ++i) {
    if (init_) {
      double diff =
          (core::XYZ(ps_[i]).get_coordinates() - last_[i]).get_magnitude();
      cur_total_move += diff;
      if (diff > max_move_) {
        max_move_ = diff;
        max_mover_ = ps_[i]->get_name();
      }
    }
    last_[i] = core::XYZ(ps_[i]).get_coordinates();
  }
  const double cur_average_move = cur_total_move / ps_.size();
  max_average_ = std::max(cur_average_move, max_average_);
  total_move_ += cur_total_move;
  total_movers_ += ps_.size();
  init_ = true;
}
void MoveStatisticsScoreState::do_after_evaluate(DerivativeAccumulator *) {}
ModelObjectsTemp MoveStatisticsScoreState::do_get_inputs() const {
  return get_as<ModelObjectsTemp>(ps_);
}
ModelObjectsTemp MoveStatisticsScoreState::do_get_outputs() const {
  return ModelObjectsTemp();
}
void MoveStatisticsScoreState::reset() {
  max_move_ = 0;
  max_average_ = 0;
  total_move_ = 0;
  total_movers_ = 0;
  last_.resize(ps_.size());
  init_ = false;
}
void MoveStatisticsScoreState::show_statistics(std::ostream &out) const {
  boost::format format("%1%: %30t%2%\n");
  out << format % "Average move" % (total_move_ / total_movers_);
  out << format % "Maximum move" % max_move_;
  out << format % "Maximum mover" % max_mover_;
  out << format % "Maximum average move" % max_average_;
}
IMPCORE_END_NAMESPACE
