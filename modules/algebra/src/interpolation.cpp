/**
 *  \file interpolation.cpp
 *  \brief Classes and operations related with interpolation in 1D and 2D
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#include "IMP/algebra/interpolation.h"

IMPALGEBRA_BEGIN_NAMESPACE

double interpolate(Matrix2D<double>&m,
                   VectorD<2>&idx,
                   bool wrap,
                   double outside,
                   int interp) {
  double result;
  switch(interp) {
    case 0:
      result=bilinear_interpolation(m,idx,wrap,outside);
      break;
    case 1:
      result=Bspline_interpolation(m,idx,wrap,outside);
      break;
  }
  return result;
}


double bilinear_interpolation(Matrix2D<double>& m,
                  VectorD<2>& idx,
                  bool wrap,
                  double outside) {
  // lower limits (xlow,ylow) are stored in v1, upper limits (xup,yup) in v2
  int v1[2], v2[2];
  double diff[2], result;

  if(!wrap) {
    // No interpolation can be done, the value is not in the image
    if(!m.is_logical_element(idx)) {
      return outside;
    } else {
      // Set the coordinates for the 4 points used in the interpolation
      for(int i=0;i<2;i++) {
        v1[i] = (int)floor(idx[i]);
        // no interpolation can be done, v1[i] is in the border of the image
        if(v1[i]==m.get_finish(i)) {
          return outside;
        } else {
          v2[i] = v1[i]+1;
        }
        diff[i] = (double)idx[i] - (double)v1[i];
      }
    }
  // Wrap is required
  } else {
    for(int i=0;i<2;i++) {
      v1[i] = (int)floor(idx[i]);
      v2[i] = v1[i]+1;
      int size = m.get_size(i);
      // this line must be before the wrapping ones
      diff[i] = (double)idx[i] - (double)v1[i];
      // wrap
      if(v1[i]<m.get_start(i) ) { v1[i]+=size; }
      if(v2[i]<m.get_start(i) ) { v2[i]+=size; }
      if(v1[i]>m.get_finish(i)) { v1[i]-=size; }
      if(v2[i]>m.get_finish(i)) { v2[i]-=size; }
    }
  }

  // Interpolate
  result= m(v1[0],v1[1])*(1-diff[0])*(1-diff[1]) +
          m(v2[0],v1[1])*(  diff[0])*(1-diff[1]) +
          m(v1[0],v2[1])*(1-diff[0])*(diff[1])   +
          m(v2[0],v2[1])*(  diff[0])*(diff[1]);

#ifdef DEBUG
  if(result>3) {
    std::cout << " v1 " << v1[0] << " " << v1[1]
              << " v2 " << v2[0] << " " << v2[1]
              << " diff " << diff[0] << " " << diff[1]
              << " dix " << idx[0] << " " << idx[1] << std::endl;
  }
#endif
  return result;
}

double Bspline_interpolation(Matrix2D<double>& m,
                  VectorD<2>& idx,
                  bool wrap,
                  double outside) {
  // TODO: Implement B-spline interpolation
  IMP_not_implemented;
  return 0.0;
}

// #undef DEBUG

IMPALGEBRA_END_NAMESPACE
