/**
 * \file Result.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_RESULT_H
#define IMP_RESULT_H

#include <IMP/base/exception.h>
#include <IMP/algebra/Transformation3D.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <vector>

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

namespace {

int read_results_file(const std::string file_name,
                      std::vector<Result>& results) {
  std::ifstream in_file(file_name.c_str());
  if (!in_file) {
    IMP_THROW("Can't open file " << file_name, IMP::base::IOException);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line);  // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("|"),
                 boost::token_compress_on);
    if (split_results.size() < 5) continue;
    int number = atoi(split_results[0].c_str());
    float score = atof(split_results[1].c_str());
    bool filtered = true;  // + value
    if (split_results[2].find("-") != std::string::npos)
      filtered = false;  // - found
    float z_score = atof(split_results[3].c_str());
    std::string transformation = split_results[split_results.size() - 1];
    boost::trim(transformation);

    // parse transformation
    std::vector<std::string> split_results2;
    boost::split(split_results2, transformation, boost::is_any_of(" "),
                 boost::token_compress_on);
    if (split_results2.size() != 6) continue;

    IMP::algebra::Rotation3D rotation =
      IMP::algebra::get_rotation_from_fixed_xyz(atof(split_results2[0].c_str()),
                                                atof(split_results2[1].c_str()),
                                                atof(split_results2[2].c_str()));
    IMP::algebra::Vector3D translation(atof(split_results2[3].c_str()),
                                       atof(split_results2[4].c_str()),
                                       atof(split_results2[5].c_str()));
    IMP::algebra::Transformation3D trans(rotation, translation);
    Result r(number, score, filtered, z_score, trans);
    results.push_back(r);
  }
  in_file.close();
  return results.size();
}

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
      std += (score*score);
    }
  }
  average /= counter;
  std /= counter;
  std -= (average*average);
  std = sqrt(std);

  // update z_scores
  for (unsigned int i = 0; i < results.size(); i++) {
    if (!results[i].is_filtered()) {
      float z_score = (results[i].get_score() - average) / std;
      results[i].set_z_score(z_score);
    }
  }
}

}

#endif /* IMP_RESULT_H */
