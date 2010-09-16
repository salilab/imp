/**
 *  \file scores2D.cpp
 *  \brief Scoring functions for 2D
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/scores2D.h"
#include "IMP/em/image_transformations.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/algebra/Transformation2D.h"

IMPEM2D_BEGIN_NAMESPACE


double average_rotation_error(RegistrationResults correct_RRs,
                              RegistrationResults computed_RRs) {
  unsigned int n_regs= std::min(correct_RRs.size(),computed_RRs.size());
  double Score=0.0;
  for (unsigned int i=0;i<n_regs ;++i ) {
    Score+=rotation_error(correct_RRs[i],computed_RRs[i]);
  }
  return Score/n_regs;
}


double rotation_error(RegistrationResult &rr1, RegistrationResult &rr2) {
  algebra::Rotation3D InvRot1 = rr1.get_rotation().get_inverse();
  algebra::Rotation3D Rot2 = rr2.get_rotation();
  algebra::Rotation3D Rot1_to_Rot2 = compose(Rot2,InvRot1);
  algebra::AxisAnglePair axang = algebra::get_axis_and_angle(Rot1_to_Rot2);
  return axang.second;
}



double average_shift_error(RegistrationResults correct_RRs,
                           RegistrationResults computed_RRs) {
  unsigned int n_regs= std::min(correct_RRs.size(),computed_RRs.size());
  double Score=0.0;
  for (unsigned int i=0;i<n_regs ;++i ) {
    Score+=shift_error(correct_RRs[i],computed_RRs[i]);
  }
  return Score/n_regs;
}

double shift_error(RegistrationResult &rr1, RegistrationResult &rr2) {
  return algebra::get_distance(rr1.get_shift(),rr2.get_shift());
}

double ccc_to_em2d(double ccc) {
  return 1-ccc;
}
double em2d_to_ccc(double em2d) {
  return 1-em2d;
}


IMPEM2D_END_NAMESPACE
