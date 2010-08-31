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


double discrepancy_scores(const em::Images &subjects,
                          const em::Images &projections,
        bool apply_transformations) {
  double Score=0.0;
  for(unsigned long i=0;i<subjects.size();++i) {
    Score +=discrepancy_score(*subjects[i],*projections[i],
                                  apply_transformations);
  }
  return Score/(double)subjects.size();
}

double discrepancy_score(em::Image &subject,em::Image &projection,
                         bool apply_transformations) {

  IMP_NEW(em::Image,img_aux,());
  //em::Image img_aux;
  em::normalize(subject);
  double ccc;
  // Transform the corresponding projection if requested
  if(apply_transformations) {
    algebra::Rotation2D   R((double)projection.get_header().get_Psi());
    algebra::Vector2D shift((double)projection.get_header().get_xorigin(),
                            (double)projection.get_header().get_yorigin());
    algebra::Transformation2D t(R,shift);
//    apply_Transformation2D(projection.get_data(),t,img_aux.get_data(),true);
    em::apply_Transformation2D(
                projection.get_data(),t,img_aux->get_data(),true);
//    em2d::normalize(img_aux,true);
    em::normalize(*img_aux,true);
//    ccc= subject.get_data().cross_correlation_coefficient(img_aux.get_data());
    ccc= subject.get_data().cross_correlation_coefficient(img_aux->get_data());
  } else {
    ccc=subject.get_data().cross_correlation_coefficient(projection.get_data());
  }
  return ccc_to_em2d_score(ccc);
}



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

IMPEM2D_END_NAMESPACE
