/**
 *  \file scores2D.cpp
 *  \brief Scoring functions for 2D
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/scores2D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/algebra/Transformation2D.h"


IMPEM2D_BEGIN_NAMESPACE


double cross_correlation_coefficient(const cv::Mat &m1,const cv::Mat &m2) {
  cv::Scalar mean1,stddev1,mean2,stddev2;
  cv::meanStdDev(m1,mean1,stddev1);
  cv::meanStdDev(m2,mean2,stddev2);
  cv::Mat cc;
  cv::multiply(m1 - mean1[0],m2-mean2[0],cc);
  cv::Scalar c = cv::sum(cc);
  double ccc = c[0]/(cc.rows*cc.cols*stddev1[0]*stddev2[0]);
  return ccc;
}



double average_rotation_error(const RegistrationResults &correct_RRs,
                              const RegistrationResults &computed_RRs) {
  unsigned int n_regs= std::min(correct_RRs.size(),computed_RRs.size());
  double Score=0.0;
  for (unsigned int i=0;i<n_regs ;++i ) {
    Score+=rotation_error(correct_RRs[i],computed_RRs[i]);
  }
  return Score/n_regs;
}


double rotation_error(const RegistrationResult &rr1,
                      const RegistrationResult &rr2) {
  algebra::Rotation3D InvRot1 = rr1.get_rotation().get_inverse();
  algebra::Rotation3D Rot2 = rr2.get_rotation();
  algebra::Rotation3D Rot1_to_Rot2 = algebra::compose(Rot2,InvRot1);
  algebra::AxisAnglePair axang = algebra::get_axis_and_angle(Rot1_to_Rot2);
 IMP_LOG(IMP::VERBOSE,
      "rotation_error: Composed rotation " << Rot1_to_Rot2 << std::endl);
  return axang.second;
}



double average_shift_error(const RegistrationResults &correct_RRs,
                           const RegistrationResults &computed_RRs) {
  unsigned int n_regs= std::min(correct_RRs.size(),computed_RRs.size());
  double Score=0.0;
  for (unsigned int i=0;i<n_regs ;++i ) {
    Score+=shift_error(correct_RRs[i],computed_RRs[i]);
  }
  return Score/n_regs;
}

double shift_error(const RegistrationResult &rr1,
                   const RegistrationResult &rr2) {
  return algebra::get_distance(rr1.get_shift(),rr2.get_shift());
}

double ccc_to_em2d(double ccc) {
  return 1-ccc;
}
double em2d_to_ccc(double em2d) {
  return 1-em2d;
}

double get_em2d_score(const RegistrationResults &RRs) {
  double em2d = 0.0;
  for (unsigned int i=0;i < RRs.size();++i) {
    em2d += ccc_to_em2d(RRs[i].get_ccc());
  }
  return em2d/RRs.size();
}




IMPEM2D_END_NAMESPACE
