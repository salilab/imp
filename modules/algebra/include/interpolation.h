/**
 *  \file interpolation.h
 *  \brief Classes and operations related with interpolation in 1D and 2D
 *  \author Javier Velazquez-Muriel
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERPOLATION_H
#define IMPALGEBRA_INTERPOLATION_H

// #define DEBUG

#include "IMP/algebra/config.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Matrix3D.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Vector3D.h"
#include <complex>

IMPALGEBRA_BEGIN_NAMESPACE

//! Simple interpolation that is only valid for values of a ranging from 0 to 1.
/**
  \return The returned value is lower+diff*(upper-lower).  (0 < diff < 1)
**/
template<typename T>
T simple_interpolate(double diff,T lower,T upper) {
  return lower+diff*(upper-lower);
}

//! Trilinear interpolation for a point using the 8 closest values in the matrix
/**
  \param[in] m the 3D Matrix
  \param[in] idx Vector with the values to interpolate
  \param[in] wrap If true, the matrix is wrapped and values from the right are
              used when the left limit is excedeed, and viceversa. Same thing
              is done in other dimensions.
  \param[in] outside Value to apply if the requested idx falls outside the
              limits of the matrix. (It is never used if wrap is requested)
**/
template<typename T>
T trilinear_interpolation(Matrix3D<T> &m,
                  Vector3D &idx,
                   bool wrap,
                   T outside) {
  // lower limits (xlow,ylow,zlow) are stored in lower,
  // upper limits (xup,yup) are stored in upper
  int lower[3], upper[3];
  double diff[3];
  T result;
  if(!wrap) {
    // No interpolation can be done, the value is not in the image
    if(!m.is_logical_element(idx)) {
      return outside;
    } else {
      // Set the coordinates for the 8 points used in the interpolation
      for(int i=0;i<3;i++) {
        lower[i] = (int)floor(idx[i]);
        // no interpolation can be done, lower[i] is in the border of the image
        if(lower[i]==m.get_finish(i)) {
          return outside;
        } else {
          upper[i] = lower[i]+1;
        }
        diff[i] = (double)idx[i] - (double)lower[i];
      }
    }
  // Wrap is required
  } else {
    for(int i=0;i<3;i++) {
      lower[i] = (int)floor(idx[i]);
      upper[i] = lower[i]+1;
      int size = m.get_size(i);
      // this line must be before the wrapping ones
      diff[i] = (double)idx[i] - (double)lower[i];
      // wrap
      if(lower[i]<m.get_start(i) ) { lower[i]+=size; }
      if(upper[i]<m.get_start(i) ) { upper[i]+=size; }
      if(lower[i]>m.get_finish(i)) { lower[i]-=size; }
      if(upper[i]>m.get_finish(i)) { upper[i]-=size; }
    }
  }
 // Interpolate
 T c00 = simple_interpolate(diff[0],m(lower[0],lower[1],lower[2]),
                                    m(upper[0],lower[1],lower[2]));
 T c10 = simple_interpolate(diff[0],m(lower[0],upper[1],lower[2]),
                                    m(upper[0],upper[1],lower[2]));
 T c01 = simple_interpolate(diff[0],m(lower[0],lower[1],upper[2]),
                                    m(upper[0],lower[1],upper[2]));
 T c11 = simple_interpolate(diff[0],m(lower[0],upper[1],upper[2]),
                                    m(upper[0],upper[1],upper[2]));
 T c0  = simple_interpolate(diff[1],c00,c10);
 T c1  = simple_interpolate(diff[1],c01,c11);
 result= simple_interpolate(diff[2],c0,c1);

#ifdef DEBUG
    std::cout << " lower " << lower[0] << " " << lower[1] << " " << lower[2]
              << " upper " << upper[0] << " " << upper[1] << " " << upper[2]
              << " diff "  << diff[0]  << " " << diff[1]  << " " << diff[2]
              << std::endl;
#endif
  return result;
}

//! General function to call in case of interpolation in 2D matrices
/**
  \param[in] m the Matrix (2D)
  \param[in] idx must be a class supporting access via [] and must have 2
              elements.
  \param[in] wrap If true, the matrix is wrapped: values from the right are
              used when the left limit is excedeed, and viceversa. Same thing
              for other dimensions.
  \param[in] outside Value to apply if the requested idx falls outside the
              limits of the matrix. (Never used if wrap is requested)
  \param[in] interp type of interpolation desired. Right now it is only
             bilinear interpolation
**/
double interpolate(algebra::Matrix2D<double> &m,
                    Vector2D& idx,
                    bool wrap = false,
                    double outside = 0.0,
                    int interp=0);


//! General function to call in case of interpolation in 3D matrices
/**
  \param[in] idx must be a class supporting access via [] and must have 2
              elements.
  \param[in] wrap If true, the matrix is wrapped: values from the right are
              used when the left limit is excedeed, and viceversa. Same thing
              for other dimensions.
  \param[in] outside Value to apply if the requested idx falls outside the
              limits of the matrix. (Never used if wrap is requested)
  \param[in] interp Interpolation method desired. Right now it is only
             trilinear interpolation
**/
template<typename T>
T interpolate(algebra::Matrix3D<T> &m,
                    Vector3D &idx,
                    bool wrap = false,
                    T outside = 0.0,
                    int interp=0) {
  T result;
  switch(interp) {
    case 0:
      result=trilinear_interpolation(m,idx,wrap,(T)outside);
      break;
    case 1:
  //TODO: Other interpolation schemes, (Bspline, gridding)
//      result=Bspline_interpolation(m,idx,wrap,outside);
      result = 0.0;
      break;
  }
  return result;
}


//! Performs bilinear interpolation for a point using the
//! 4 closest values in the matrix
//TODO: Make a templated version of the bilinear interpolation
/**
  \param[in] m the Matrix (2D)
  \param[in] idx must be a class supporting access via []. It must have 2
              elements.
  \param[in] wrap if true, the image is wrapped and values from the right are
              used when the left limit is excedeed, and viceversa. Same thing
              is done between top and bottom.
  \param[in] outside Value to apply if the requested idx falls outside the
              limits of the matrix. (It is never used if wrap is requested)
**/
IMPALGEBRAEXPORT double bilinear_interpolation(Matrix2D<double>& m,
                  Vector2D& idx,
                  bool wrap = false,
                  double outside = 0.0);

IMPALGEBRAEXPORT double Bspline_interpolation(Matrix2D<double>& m,
                  Vector2D& idx,
                  bool wrap = false,
                  double outside = 0.0);

// #undef DEBUG

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_INTERPOLATION_H */
