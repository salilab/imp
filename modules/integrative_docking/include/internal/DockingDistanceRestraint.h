/**
 * \file IMP/integrative_docking/DockingDistanceRestraint.h \brief
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_DOCKING_DISTANCE_RESTRAINT_H
#define IMPINTEGRATIVE_DOCKING_DOCKING_DISTANCE_RESTRAINT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/utility.h>
#include <IMP/constants.h>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class DockingDistanceRestraint {
 public:
  DockingDistanceRestraint() {}
  DockingDistanceRestraint(IMP::algebra::Vector3D receptor_point,
                           IMP::algebra::Vector3D ligand_point,
                           float max_distance, float min_distance = 0.0,
                           float weight = 1.0)
      : receptor_point_(receptor_point),
        ligand_point_(ligand_point),
        min_distance_(min_distance),
        max_distance_(max_distance),
        min_distance_squared_(min_distance * min_distance),
        max_distance_squared_(max_distance * max_distance),
        weight_(weight) {
    mean_ = (max_distance_ - min_distance_) / 2.0;
    sd_ = (max_distance_ - mean_) / 3.0;
    // std::cerr << "mean " << mean_ << " sd " << sd_ << std::endl;
  }

  bool is_satisfied(const IMP::algebra::Transformation3D& t) const {
    float squared_distance =
        IMP::algebra::get_squared_distance(receptor_point_, t * ligand_point_);
    return (squared_distance >= min_distance_squared_ &&
            squared_distance <= max_distance_squared_);
  }

  float get_distance() const {
    return IMP::algebra::get_distance(receptor_point_, ligand_point_);
  }

  float get_distance(const IMP::algebra::Transformation3D& t) const {
    return IMP::algebra::get_distance(receptor_point_, t * ligand_point_);
  }

  float get_weight() const { return weight_; }

  float get_score(float squared_distance) const {
    static float constant = 1.0 / (sd_ * sqrt(2 * IMP::PI));
    if (squared_distance >= min_distance_squared_ &&
        squared_distance <= max_distance_squared_) {
      float distance = sqrt(squared_distance);
      float score =
          constant *
          std::exp(-0.5 * IMP::square((distance - mean_) / sd_));
      return score;
    }
    return 0.0;
  }

  float get_score() const {
    return get_score(
        IMP::algebra::get_squared_distance(receptor_point_, ligand_point_));
  }

  float get_score(const IMP::algebra::Transformation3D& t) const {
    return get_score(
        IMP::algebra::get_squared_distance(receptor_point_, t * ligand_point_));
  }

  friend std::ostream& operator<<(std::ostream& s,
                                  const DockingDistanceRestraint& d) {
    return s << d.receptor_point_ << ' ' << d.ligand_point_ << ' '
             << d.max_distance_;
  }

 public:
  IMP::algebra::Vector3D receptor_point_;
  IMP::algebra::Vector3D ligand_point_;
  float min_distance_;
  float max_distance_;
  float min_distance_squared_;
  float max_distance_squared_;
  float weight_;
  float mean_;
  float sd_;
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_DOCKING_DISTANCE_RESTRAINT_H */
