/**
 *  \file align2D.h
 *  \brief Alignment of images in 2D
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_ALIGN_2D_H
#define IMPEM2D_ALIGN_2D_H

#include "IMP/em2d/PolarResamplingParameters.h"
#include "IMP/em2d/em2d_config.h"
#include "IMP/em/image_transformations.h"
#include "IMP/algebra/utility.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Transformation2D.h"
#include "IMP/algebra/Vector2D.h"
#include <IMP/VectorOfRefCounted.h>
#include <algorithm>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

typedef std::vector< std::vector<double> > double_rings;
typedef std::vector< std::complex<double> > complex_ring;
typedef std::vector< complex_ring > complex_rings;
// typedef std::vector< std::vector< std::complex<double> > > complex_rings;
typedef std::pair<algebra::Transformation2D,double> ResultAlign2D;


//! Aligns completely two matrices (rotationally and translationally).
//! Based on Frank, 2006, pg. 99, and Penczek, Ultram.,1992. Uses the
//! autocorrelation function
/**
  \param[in] m1 first matrix. Used as reference
  \param[in] m2 the matrix that is aligned with m1
  \param[out] t the resulting transformation to apply to m2 to align it with m1
  \param[out] The value of the maximum cross correlation coefficient
**/
IMPEM2DEXPORT ResultAlign2D align2D_complete(algebra::Matrix2D_d &m1,
          algebra::Matrix2D_d &m2,bool apply=false,
          int interpolation_method=0);


//! Aligns two matrices rotationally.
//! Based on polar resampling.
/**
  \param[in] m1 first matrix. Used as reference
  \param[in] m2 the matrix that is aligned with m1
  \param[out] angle the resulting angle to apply to m2 to align it with m1
  \param[in] apply Set it to true if you want the rotation applied to m2
  \param[in] pad set it to false if the matrices are already padded
  \param[out] Vector2D containing the tupe (angle,cross correlation)
  \note: The cross correlation is not normalized
**/
IMPEM2DEXPORT ResultAlign2D align2D_rotational(algebra::Matrix2D_d &m1,
        algebra::Matrix2D_d &m2, bool apply=false,
        bool pad=false,int interpolation_method=0);

//! Aligns two matrices (translationally)
/**
  \param[in] m1 first matrix.
  \param[in] m2 the matrix that is aligned with the reference
  \param[in] apply Set it to true if you want the rotation applied to m2
  \param[in] pad set it to false if the matrices are already padded
  \param[out] Vector3D containing the values (i,j,cross correlation), where i,j
              are the pixels of peak value (they are double).
**/
IMPEM2DEXPORT ResultAlign2D align2D_translational(algebra::Matrix2D_d &m1,
                                       algebra::Matrix2D_d &m2,
                                       bool apply=false,
                                       bool pad=false);







IMPEM2DEXPORT double align2D_complete_no_preprocessing(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_c &M1,
                      complex_rings &fft_rings1,
                      algebra::Matrix2D_d &m2,
                      complex_rings &fft_rings2,
                      algebra::Transformation2D &t,
                      bool apply);

//! Makes the computations for rotationally align in 2D, but does not do
//! any preprocessing. Every input needed must be provided
IMPEM2DEXPORT double align2D_rotational_no_preprocessing(
                          complex_rings &fft_rings1,
                          complex_rings &fft_rings2,
                          double *angle,
                          const PolarResamplingParameters polar_params);


//! Aligns two matrices (translationally) using the convolution theorem to
//! compute the cross correlation function in Fourier space.
/**
  \param[in] M1 FFT transform of the first matrix
  \param[in] M2 FFT transform of the second matrix
  \param[in] v Vector with the solution
  \param[out] The value of the maximum cross correlation
**/
IMPEM2DEXPORT double align2D_translational_no_preprocessing(
               algebra::Matrix2D_c &M1,
               algebra::Matrix2D_c &M2,
               algebra::Vector2D &v,
               unsigned int original_rows,
               unsigned int original_cols);



IMPEM2DEXPORT complex_rings preprocess_for_align2D_rotational(
      algebra::Matrix2D_d &m,bool dealing_with_subjects=false,
      int interpolation_method=0);


//! Resamples a matrix to polar coordinates. Allows for variable number of
//! points
/**
  \param[in] f matrix to resample
  \param[out] rings resampled rings, from inner to outer
  \param[in] dealing_with_subjects if true, resamples the matrix as
              radius*m(radius,angle). This is only useful for rotational
              alignments. False (default) gives the regular polar resampling
  \param[in] interp type of interpolation required. See interpolation.h in
             algebra module for further help.
**/
IMPEM2DEXPORT void resample2D_polar(algebra::Matrix2D_d &f,
                                    double_rings &rings,
                                    bool dealing_with_subjects=false,
                                    int interpolation_method=0);


IMPEM2DEXPORT void resample2D_polar(algebra::Matrix2D_d &m,
                      algebra::Matrix2D_d &result,
                      int interpolation_method=0);

//! Performs a peak search in a Matrix2D
//! Useful for cross correlation peak search
IMPEM2DEXPORT algebra::Vector2D peak_search(
                                      algebra::Matrix2D_d &m,double *value);



//! Performs a peak search in a vector class
/**
  \param[in] v the class where to perform the search
  \param[in] len the length of the class (number of elements)
**/
IMPEM2DEXPORT double peak_search(std::vector<double> &v,double *value);


//! computes the center of gravity for a subpart of the Matrix2D.
//! The weights are the matrix values
/**
 \param[out] c class to store the c.o.g. It must be a class
 admiting access via []
 \param[in] dims vector with the list of sizes for the subpart. If NULL,
 the entire matrix is taken
 \param[in] inds class with the list of starting indices for the subpart. If
 NULL, the beginning of the matrix is taken
**/

IMPEM2DEXPORT algebra::Vector2D compute_center_of_gravity(
                        algebra::Matrix2D_d &m,
                        int *dims=NULL,int *bases=NULL);



IMPEM2DEXPORT void print_vector(std::vector< std::complex<double> > &v);


//! Aligns completely two matrices (rotationally and translationally).
//! Using centers
/**
  \param[in] m1 first matrix. Used as reference
  \param[in] m2 the matrix that is aligned with m1
  \param[out] t the resulting transformation to apply to m2 to align it with m1
  \param[out] The value of the maximum cross correlation coefficient
**/
IMPEM2DEXPORT double align2D_complete_with_centers(algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_d &m2,
                      algebra::Transformation2D &t, bool apply,
                      int interpolation_method);



IMPEM2DEXPORT double align2D_complete_with_centers_no_preprocessing(
                      algebra::Vector2D &center1,
                      algebra::Vector2D &center2,
                      complex_rings &fft_rings1,
                      complex_rings &fft_rings2,
                      algebra::Transformation2D &t,
                      PolarResamplingParameters polar_params);


IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_ALIGN_2D_H */
