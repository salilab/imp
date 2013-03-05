/**
 *  \file scores2D.cpp
 *  \brief Scoring functions for 2D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/scores2D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/algebra/Transformation2D.h"


IMPEM2D_BEGIN_NAMESPACE


double get_cross_correlation_coefficient(const cv::Mat &m1,
                                         const cv::Mat &m2) {
  cv::Scalar mean1,stddev1,mean2,stddev2;
  my_meanStdDev(m1,mean1,stddev1); // cv::meanStdDev(m1,mean1,stddev1);
  my_meanStdDev(m2,mean2,stddev2); // cv::meanStdDev(m2,mean2,stddev2);
  cv::Mat cc;
  cv::multiply(m1 - mean1[0],m2-mean2[0],cc);
  cv::Scalar c = cv::sum(cc);
  double ccc = c[0]/(cc.rows*cc.cols*stddev1[0]*stddev2[0]);
  return ccc;
}



double get_average_rotation_error(const RegistrationResults &correct_RRs,
                              const RegistrationResults &computed_RRs) {
  unsigned int n_regs= std::min(correct_RRs.size(),computed_RRs.size());
  double Score=0.0;
  for (unsigned int i=0;i<n_regs ;++i ) {
    Score+=get_rotation_error(correct_RRs[i],computed_RRs[i]);
  }
  return Score/n_regs;
}


double get_rotation_error(const RegistrationResult &rr1,
                      const RegistrationResult &rr2) {
  algebra::Rotation3D InvRot1 = rr1.get_rotation().get_inverse();
  algebra::Rotation3D Rot2 = rr2.get_rotation();
  algebra::Rotation3D Rot1_to_Rot2 = algebra::compose(Rot2,InvRot1);
  algebra::AxisAnglePair axang = algebra::get_axis_and_angle(Rot1_to_Rot2);
 IMP_LOG_VERBOSE(
      "get_rotation_error: Composed rotation " << Rot1_to_Rot2 << std::endl);
  return axang.second;
}



double get_average_shift_error(const RegistrationResults &correct_RRs,
                           const RegistrationResults &computed_RRs) {
  unsigned int n_regs= std::min(correct_RRs.size(),computed_RRs.size());
  double Score=0.0;
  for (unsigned int i=0;i<n_regs ;++i ) {
    Score+=get_shift_error(correct_RRs[i],computed_RRs[i]);
  }
  return Score/n_regs;
}

double get_shift_error(const RegistrationResult &rr1,
                   const RegistrationResult &rr2) {
  return algebra::get_distance(rr1.get_shift(),rr2.get_shift());
}


double get_global_score(const RegistrationResults &RRs) {

  double global_score = 0.0;
  for (unsigned int i=0;i < RRs.size();++i) {
    global_score += RRs[i].get_score();
  }
  return global_score/RRs.size();
}


double MeanAbsoluteDifference::get_private_score(Image *image,
                                               Image *projection) const {
  double result =0.0;
  cvDoubleConstMatIterator img_it = image->get_data().begin<double>();
  cvDoubleConstMatIterator prj_it = projection->get_data().begin<double>();
  double n = 0;
  for( ; img_it != image->get_data().end<double>() ||
         prj_it != projection->get_data().end<double>() ; ++img_it, ++prj_it ) {
    result += std::abs(*img_it - *prj_it);
    n += 1.0;
  }
  result = result / n;
  return result;
}

double ChiSquaredScore::get_private_score(Image *image,
                                          Image *projection) const {
  double result = 0.0;
  cvDoubleConstMatIterator img_it = image->get_data().begin<double>();
  cvDoubleConstMatIterator prj_it = projection->get_data().begin<double>();
  cvDoubleConstMatIterator var_it = variance_->get_data().begin<double>();
  double n = 0;
  for( ; img_it != image->get_data().end<double>() ||
         prj_it != projection->get_data().end<double>() ;
         ++img_it, ++prj_it, ++var_it ) {
    double aux = (*img_it - *prj_it);
    result += aux * aux / (*var_it);
    n += 1.0;
  }
  result = result / n;
  return result;


}

IMPEM2D_END_NAMESPACE
