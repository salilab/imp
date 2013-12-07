/**
 *  \file SurfaceMatching.h   \brief Surface matching.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SURFACE_MATCHING_H
#define IMPMULTIFIT_SURFACE_MATCHING_H

#include "IMP/algebra/standard_grids.h"
#include "IMP/algebra/Transformation3D.h"
#include <IMP/multifit/multifit_config.h>

IMPMULTIFIT_BEGIN_INTERNAL_NAMESPACE

class IMPMULTIFITEXPORT CandidateSurfaceMatchingParameters {
  double ball_radius_;
  double maximum_pair_distance_;
  double maximum_normal_error_;
  double maximum_torsion_error_;
  double maximum_pair_distance_error_;
  double maximum_angle_error_;

 public:
  CandidateSurfaceMatchingParameters()
      : ball_radius_(6),
        maximum_pair_distance_(20),
        maximum_normal_error_(1.6),
        maximum_torsion_error_(2.1),
        maximum_pair_distance_error_(2.0),
        maximum_angle_error_(0.9) {}

  double get_ball_radius() const { return ball_radius_; }

  void set_ball_radius(double radius) { ball_radius_ = radius; }

  double get_maximum_pair_distance() const { return maximum_pair_distance_; }

  void set_maximum_pair_distance(double distance) {
    maximum_pair_distance_ = distance;
  }

  double get_maximum_normal_error() const { return maximum_normal_error_; }

  void set_maximum_normal_error(double error) { maximum_normal_error_ = error; }

  double get_maximum_torsion_error() const { return maximum_torsion_error_; }

  void set_maximum_torsion_error(double error) {
    maximum_torsion_error_ = error;
  }

  double get_maximum_angle_error() const { return maximum_angle_error_; }

  void set_maximum_angle_error(double error) { maximum_angle_error_ = error; }

  double get_maximum_pair_distance_error() const {
    return maximum_pair_distance_error_;
  }

  void set_maximum_pair_distance_error(double error) {
    maximum_pair_distance_error_ = error;
  }
};

IMPMULTIFITEXPORT IMP::algebra::Transformation3Ds
    get_candidate_transformations_match_1st_surface_to_2nd(
        const IMP::algebra::DenseGrid3D<float> &map0,
        const IMP::algebra::DenseGrid3D<float> &map1,
        const CandidateSurfaceMatchingParameters &params =
            CandidateSurfaceMatchingParameters());

IMPMULTIFIT_END_INTERNAL_NAMESPACE

#endif /* IMPMULTIFIT_SURFACE_MATCHING_H */
