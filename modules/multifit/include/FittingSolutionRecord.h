/**
 *  \file IMP/multifit/FittingSolutionRecord.h
 *  \brief stored a multifit fitting solution
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_FITTING_SOLUTION_RECORD_H
#define IMPMULTIFIT_FITTING_SOLUTION_RECORD_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/kernel/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/multifit/multifit_config.h>
IMPMULTIFIT_BEGIN_NAMESPACE

//! A fitting solution record
/**
\todo discuss with Daniel - should it be an object ?
 */
class IMPMULTIFITEXPORT FittingSolutionRecord {
 public:
  //! Constructor
  /**
  \todo consider using better initialization
  */
  FittingSolutionRecord();
  inline unsigned int get_index() const { return index_; }
  void set_index(unsigned int new_ind) { index_ = new_ind; }
  inline std::string get_solution_filename() const { return sol_fn_; }
  void set_solution_filename(std::string sol_fn) { sol_fn_ = sol_fn; }
  inline algebra::Transformation3D get_fit_transformation() const {
    return fit_transformation_;
  }
  void set_fit_transformation(algebra::Transformation3D t) {
    fit_transformation_ = t;
  }
  inline unsigned int get_match_size() const { return match_size_; }
  void set_match_size(unsigned int match_size) { match_size_ = match_size; }
  inline Float get_match_average_distance() const { return match_avg_dist_; }
  void set_match_average_distance(Float match_avg_dist) {
    match_avg_dist_ = match_avg_dist;
  }
  inline Float get_fitting_score() const { return fitting_score_; }
  void set_fitting_score(Float fit_score) { fitting_score_ = fit_score; }
  inline Float get_rmsd_to_reference() const { return rmsd_to_ref_; }
  void set_rmsd_to_reference(Float rmsd_to_ref) { rmsd_to_ref_ = rmsd_to_ref; }
  inline algebra::Transformation3D get_dock_transformation() const {
    return dock_transformation_;
  }
  void set_dock_transformation(algebra::Transformation3D t) {
    dock_transformation_ = t;
  }
  inline Float get_envelope_penetration_score() const { return env_pen_; }
  void set_envelope_penetration_score(Float s) { env_pen_ = s; }
  //! Show
  /**
  \todo consider using initialization mechanism
  */
  void show(std::ostream& out = std::cout) const;
  static std::string get_record_header() {
    std::stringstream ss;
    ss << "solution index | solution filename | fit rotation | fit translation "
          " |"
       << " match size | match average distance | "
       << " envelope penetration score | fitting score|"
       << "dock rotation | dock translation |"
       << " RMSD to reference" << std::endl;
    return ss.str();
  }

 protected:
  unsigned int index_;
  std::string sol_fn_;
  algebra::Transformation3D fit_transformation_;   // fit to map
  algebra::Transformation3D dock_transformation_;  // best geo dock to partners
  unsigned int match_size_;
  Float match_avg_dist_;
  Float env_pen_;
  Float fitting_score_;
  Float rmsd_to_ref_;
};
IMP_VALUES(FittingSolutionRecord, FittingSolutionRecords);

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_FITTING_SOLUTION_RECORD_H */
