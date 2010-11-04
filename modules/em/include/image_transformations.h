/**
 *  \file image_transformations.h
 *  \brief transformations for EM images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPEM_IMAGE_TRANSFORMATIONS_H
#define IMPEM_IMAGE_TRANSFORMATIONS_H

#include "IMP/em/em_config.h"
#include "IMP/em/Image.h"
#include "IMP/algebra/Vector2D.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/algebra/Transformation2D.h"
#include "IMP/algebra/interpolation.h"


IMPEM_BEGIN_NAMESPACE

//! Applies a transformation to a Matrix2D.
//! The matrix is first rotated and then translated.
/**
  \param[in] m the matrix to rotate
  \param[in] t The transformation to apply. The translation in t is
             understood as (rows,cols).
  \param[out] result the result matrix. It cannot be input matrix.
  \param[in] wrap if true, the Matrix2D is wrapped.
                      See help for bilinear_interp()
  \param[in] outside Value to apply if a rotated point falls outside the
               limits of the matrix. (default = 0.0)
  \param[in] interp type of interpolation required. See interpolation.h in
             algebra module for further help.
**/
IMPEMEXPORT void apply_Transformation2D(algebra::Matrix2D_d &m,
              const algebra::Transformation2D &t,
              algebra::Matrix2D_d &result,
              bool wrap=false,double outside=0.0,int interp = 0);


//! Shifts (physically) the values of the Matrix2D a vector v.
/**
  \note The values are physically shifted. That is, the
  matrix will still output the same values for get_start() and get_size()
  but the values will be changed.
  \param[in] m Matrix of the image to be shifted
  \param[in] v Shift vector
  \param[in] result A matrix where to store the result. It cannot be the
              same as the input.
  \param[in] wrap If true, wrapping of the values is done in all dimensions
              when shifting. If false, outside value is used instead
  \param[in] outside Value to apply for the new pixels of the Matrix2D
              when shifting (It is never used if wrap is true)
  \param[in] interp. Interpolation method used
**/
IMPEMEXPORT void shift(algebra::Matrix2D_d &m,
          const algebra::Vector2D &v,
           algebra::Matrix2D_d &result,
           bool wrap=true,double outside=0.0,int interp=0);

//! Rotates a Matrix2D a given angle (in radians). The center of the matrix
//! is used as the center of rotation. The input matrix is not modified.
/**
  \param[in] m the matrix to rotate
  \param[out] result the result matrix. It cannot be input matrix.
  \param[in] ang angle
  \param[in] wrap if true, the Matrix2D is wrapped.
                      See help for bilinear_interp()
  \param[in] outside Value to apply if a rotated point falls outside the
               limits of the matrix. (default = 0.0)
  \param[in] interp type of interpolation required. See interpolation.h in
             algebra module for further help.
**/
IMPEMEXPORT void rotate(algebra::Matrix2D_d &m,double ang,
              algebra::Matrix2D_d &result,
              bool wrap=false,double outside=0.0,int interp = 0);


//! Normalizes an image so the mean is 0 and the standard deviation is 1.
/**
  \param[in] im the image
  \param[in] force if True, the normalization is performed again even if the
             header of the image says is already normalized. Only use this
             option if you know what you are doing
  \note the max, min, average and standard deviation values of the header are
  modified accordingly.
**/
IMPEMEXPORT void normalize(em::Image *im,bool force=false);

IMPEM_END_NAMESPACE
#endif  /* IMPEM_IMAGE_TRANSFORMATIONS_H */
