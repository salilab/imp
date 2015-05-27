/**
 * \file IMP/integrative_docking/Result.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_RESULT_H
#define IMPINTEGRATIVE_DOCKING_RESULT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/exception.h>
#include <IMP/algebra/Transformation3D.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <vector>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class Result {
 public:
  Result(int number, float score, bool filtered, float z_score,
         IMP::algebra::Transformation3D transformation =
             IMP::algebra::get_identity_transformation_3d())
      : number_(number),
        score_(score),
        filtered_(filtered),
        z_score_(z_score),
        transformation_(transformation) {}

  int get_number() const { return number_; }
  float get_score() const { return score_; }
  bool is_filtered() const { return filtered_; }
  float get_z_score() const { return z_score_; }
  IMP::algebra::Transformation3D get_transformation() const {
    return transformation_;
  }

  void set_score(float score) { score_ = score; }
  void set_z_score(float z_score) { z_score_ = z_score; }

  friend std::ostream& operator<<(std::ostream& s, const Result& p) {
    s.width(6);
    s << p.number_ << " | ";
    s.precision(3);
    s.width(6);
    s << p.score_;
    if (p.filtered_)
      s << " |  -   | ";
    else
      s << " |  +   | ";
    s.width(6);
    s << p.z_score_ << " | ";
    IMP::algebra::FixedXYZ euler_angles =
        IMP::algebra::get_fixed_xyz_from_rotation(
            p.transformation_.get_rotation());
    IMP::algebra::Vector3D translation = p.transformation_.get_translation();
    s.precision(4);
    s << euler_angles.get_x() << " " << euler_angles.get_y() << " "
      << euler_angles.get_z() << " " << translation[0] << " " << translation[1]
      << " " << translation[2];
    return s;
  }

 protected:
  int number_;
  float score_;
  bool filtered_;  // true when the Result is good (+ value), passes filters
  float z_score_;
  IMP::algebra::Transformation3D transformation_;
};

int read_results_file(const std::string file_name,
                      std::vector<Result>& results);

template <class ResultT>
void set_z_scores(std::vector<ResultT>& results) {
  // compute z_score
  float average = 0.0;
  float std = 0.0;
  int counter = 0;
  for (unsigned int i = 0; i < results.size(); i++) {
    if (!results[i].is_filtered()) {
      counter++;
      float score = results[i].get_score();
      average += score;
      std += (score * score);
    }
  }
  average /= counter;
  std /= counter;
  std -= (average * average);
  std = sqrt(std);

  // update z_scores
  for (unsigned int i = 0; i < results.size(); i++) {
    if (!results[i].is_filtered()) {
      float z_score = (results[i].get_score() - average) / std;
      results[i].set_z_score(z_score);
    }
  }
}


IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_RESULT_H */
