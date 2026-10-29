/**
 *  \file scores2D.cpp
 *  \brief Scoring functions for 2D
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/scores2D.h"
#include "IMP/em2d/internal/image_processing_helper.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/algebra/Transformation2D.h"

IMPEM2D_BEGIN_NAMESPACE

double get_cross_correlation_coefficient(const DoubleMatrix &m1,
                                         const DoubleMatrix &m2) {
  IMP_USAGE_CHECK(m1.rows() == m2.rows() && m1.cols() == m2.cols(),
                  "em2d:get_cross_correlation_coefficient: Matrices have "
                  "different size.");
  double mean1, stddev1, mean2, stddev2;
  internal::get_mean_and_stddev(m1, mean1, stddev1);
  internal::get_mean_and_stddev(m2, mean2, stddev2);
  double c = 0.;
  for (Eigen::Index i = 0; i < m1.size(); ++i) {
    c += (m1.data()[i] - mean1) * (m2.data()[i] - mean2);
  }
  double ccc = c / (static_cast<int>(m1.rows() * m1.cols()) * stddev1 *
                    stddev2);
  return ccc;
}

double get_average_rotation_error(const RegistrationResults &correct_RRs,
                                  const RegistrationResults &computed_RRs) {
  unsigned int n_regs = std::min(correct_RRs.size(), computed_RRs.size());
  double Score = 0.0;
  for (unsigned int i = 0; i < n_regs; ++i) {
    Score += get_rotation_error(correct_RRs[i], computed_RRs[i]);
  }
  return Score / n_regs;
}

double get_rotation_error(const RegistrationResult &rr1,
                          const RegistrationResult &rr2) {
  algebra::Rotation3D InvRot1 = rr1.get_rotation().get_inverse();
  algebra::Rotation3D Rot2 = rr2.get_rotation();
  algebra::Rotation3D Rot1_to_Rot2 = algebra::compose(Rot2, InvRot1);
  algebra::AxisAnglePair axang = algebra::get_axis_and_angle(Rot1_to_Rot2);
  IMP_LOG_VERBOSE("get_rotation_error: Composed rotation " << Rot1_to_Rot2
                                                           << std::endl);
  return axang.second;
}

double get_average_shift_error(const RegistrationResults &correct_RRs,
                               const RegistrationResults &computed_RRs) {
  unsigned int n_regs = std::min(correct_RRs.size(), computed_RRs.size());
  double Score = 0.0;
  for (unsigned int i = 0; i < n_regs; ++i) {
    Score += get_shift_error(correct_RRs[i], computed_RRs[i]);
  }
  return Score / n_regs;
}

double get_shift_error(const RegistrationResult &rr1,
                       const RegistrationResult &rr2) {
  return algebra::get_distance(rr1.get_shift(), rr2.get_shift());
}

double get_global_score(const RegistrationResults &RRs) {

  double global_score = 0.0;
  for (unsigned int i = 0; i < RRs.size(); ++i) {
    global_score += RRs[i].get_score();
  }
  return global_score / RRs.size();
}

double MeanAbsoluteDifference::get_private_score(Image *image,
                                                 Image *projection) const {
  const DoubleMatrix &img = image->get_data();
  const DoubleMatrix &prj = projection->get_data();
  IMP_USAGE_CHECK(img.size() == prj.size(),
                  "MeanAbsoluteDifference: Images have different size");
  double result = 0.0;
  double n = 0;
  for (Eigen::Index i = 0; i < img.size(); ++i) {
    result += std::abs(img.data()[i] - prj.data()[i]);
    n += 1.0;
  }
  result = result / n;
  return result;
}

double ChiSquaredScore::get_private_score(Image *image,
                                          Image *projection) const {
  const DoubleMatrix &img = image->get_data();
  const DoubleMatrix &prj = projection->get_data();
  const DoubleMatrix &var = variance_->get_data();
  IMP_USAGE_CHECK(img.size() == prj.size() && img.size() == var.size(),
                  "ChiSquaredScore: Images have different size");
  double result = 0.0;
  double n = 0;
  for (Eigen::Index i = 0; i < img.size(); ++i) {
    double aux = (img.data()[i] - prj.data()[i]);
    result += aux * aux / var.data()[i];
    n += 1.0;
  }
  result = result / n;
  return result;
}

IMPEM2D_END_NAMESPACE
