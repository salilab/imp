/**
 *  \file image_transformations.cpp
 *  \brief transformations for EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em/image_transformations.h"

IMPEM_BEGIN_NAMESPACE

void apply_Transformation2D(algebra::Matrix2D_d &m,
              const algebra::Transformation2D &t,
              algebra::Matrix2D_d &result,
              bool wrap,double outside,int interp) {
  result.resize(m);
  //result.set_zero();
  // Save the origin and center
  std::vector<int> orig(2);
  orig[0]=m.get_start(0);
  orig[1]=m.get_start(1);
  m.centered_start();
  result.centered_start();
  // Build the rotation needed for the angle (build the inverse directly)
  algebra::Rotation2D irot = t.get_rotation().get_inverse();
  // Build the shift (for images the translation in
  // Transformation2D is understood in pixels)
  algebra::Vector2D shift = t.get_translation();
  // Roll over the destination matrix
  for(int i = result.get_start(0);i<=result.get_finish(0);i++) { // Y
    for(int j = result.get_start(1);j<=result.get_finish(1);j++) { // X
      // Compute the equivalent point in the original matrix
      algebra:: Vector2D p=
              irot.get_rotated((double)i-shift[0],(double)j-shift[1]);
      // Interpolate the value from the original matrix
      result(i,j) = algebra::interpolate(m,p,wrap,outside,interp);
    }
  }
  // Restore the origins
  m.set_start(orig);
  result.set_start(orig);
}

void shift(algebra::Matrix2D_d &m,
          const algebra::Vector2D &v,
           algebra::Matrix2D_d &result,
           bool wrap,double outside,int interp) {
  algebra::Transformation2D t(algebra::get_identity_rotation_2d(),v);
  apply_Transformation2D(m,t,result,wrap,outside,interp);
}

void rotate(algebra::Matrix2D_d &m,   double ang,
            algebra::Matrix2D_d &result,
            bool wrap,
            double outside,
            int interp) {

  algebra::Rotation2D R(ang);
  algebra::Vector2D v = algebra::get_zero_vector_d<2>();
  algebra::Transformation2D t(R,v);
  apply_Transformation2D(m,t,result,wrap,outside,interp);
}

void normalize(em::Image *im,bool force) {
  if(!im->get_header().is_normalized() || force==true ) {
    im->get_data().normalize();
    im->get_header().set_fImami(1);
    im->get_header().set_fAv(0.0);
    im->get_header().set_fSig(1.0);
    im->get_header().set_fFmin(im->get_data().compute_min());
    im->get_header().set_fFmax(im->get_data().compute_max());
  }
}

IMPEM_END_NAMESPACE
