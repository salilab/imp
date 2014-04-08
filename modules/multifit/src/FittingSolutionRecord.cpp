/**
 *  \file FittingSolutionRecord.cpp
 *  \brief stored a multifit fitting solution
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/FittingSolutionRecord.h>
#include <limits>

namespace {
template <class T>
void initialize(T &t) {
  if (std::numeric_limits<T>::has_signaling_NaN) {
    t = std::numeric_limits<T>::signaling_NaN();
  } else if (std::numeric_limits<T>::has_quiet_NaN) {
    t = std::numeric_limits<T>::quiet_NaN();
  } else if (std::numeric_limits<T>::has_infinity) {
    t = std::numeric_limits<T>::infinity();
  } else {
    // numerical limits for int and double have completely
    // different meanings of max/min
    t = std::numeric_limits<T>::max();
  }
}
template <class T>
bool is_initialized(T &t) {
  if (std::numeric_limits<T>::has_signaling_NaN) {
    return !(t == std::numeric_limits<T>::signaling_NaN());
  } else if (std::numeric_limits<T>::has_quiet_NaN) {
    return !(t == std::numeric_limits<T>::quiet_NaN());
  } else if (std::numeric_limits<T>::has_infinity) {
    return !(t == std::numeric_limits<T>::infinity());
  } else {
    // numerical limits for int and double have completely
    // different meanings of max/min
    return !(t == (std::numeric_limits<T>::max()));
  }
}
}
IMPMULTIFIT_BEGIN_NAMESPACE
FittingSolutionRecord::FittingSolutionRecord() {
  index_ = 0;
  sol_fn_ = "";
  match_size_ = 0;
  match_avg_dist_ = -1;
  fitting_score_ = -1;
  rmsd_to_ref_ = -1;
  env_pen_ = -1;
  fit_transformation_ = algebra::get_identity_transformation_3d();
  dock_transformation_ = algebra::get_identity_transformation_3d();
}

void FittingSolutionRecord::show(std::ostream &out) const {
  // if (is_initialized(index_)) out<<index_;
  if (true) out << index_;
  out << "|";
  // if (is_initialized(sol_fn_)) out<<sol_fn_;
  if (true) out << sol_fn_;
  out << "|";
  fit_transformation_.get_rotation().show(out);
  out << "|";
  fit_transformation_.get_translation().show(out, " ", false);
  out << "|";
  // if (is_initialized(match_size_)) out<<match_size_;
  if (true) out << match_size_;
  out << "|";
  // if (is_initialized(match_avg_dist_)) out<<match_avg_dist_;
  if (true) out << match_avg_dist_;
  out << "|";
  if (true) out << env_pen_;
  out << "|";
  // if (is_initialized(fitting_score_)) out<<fitting_score_;
  if (true) out << fitting_score_;
  out << "|";
  dock_transformation_.get_rotation().show(out);
  out << "|";
  dock_transformation_.get_translation().show(out, " ", false);
  out << "|";
  // if (is_initialized(rmsd_to_ref_)) out<<rmsd_to_ref_;
  if (true) out << rmsd_to_ref_;
}
/*
FittingSolutionRecords generate_fitting_records(
  const em::FittingSolutions &sols) {
  FittingSolutionRecords fit_records;
  for(int i=0;i<sols.get_number_of_solutions();i++) {
    FittingSolutionRecord rec;
    rec.set_transformation(sols.get_transformation(i));
    rec.set_fitting_score(sols.get_score(i));
    fit_records.push_back(rec);
  }
  return fit_records;
}
*/

IMPMULTIFIT_END_NAMESPACE
