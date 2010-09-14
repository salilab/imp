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
  \param[in] m1 Reference matrix
  \param[in] m2 Matrix to align to the reference
  \param[in] apply if true, apply the transformation to m2 after alignment
  \param[in] interpolation_method interpolation method during resamplings,
              rotations and translations. Default:0, bilinear
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the cross correlation
**/
IMPEM2DEXPORT ResultAlign2D align2D_complete(algebra::Matrix2D_d &m1,
          algebra::Matrix2D_d &m2,bool apply=false,
          int interpolation_method=0);


//! Aligns two matrices rotationally. Based on polar resampling.
/**
  \param[in] m1 Reference matrix
  \param[in] m2 Matrix to align to the reference
  \param[in] apply if true, apply the transformation to m2 after alignment
  \param[in] interpolation_method interpolation method during resamplings,
              rotations and translations. Default:0, bilinear
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the cross correlation
  \note: The cross correlation is not normalized
**/
IMPEM2DEXPORT ResultAlign2D align2D_rotational(algebra::Matrix2D_d &m1,
        algebra::Matrix2D_d &m2, bool apply=false,
        int interpolation_method=0);

//! Aligns two matrices translationally
/**
  \param[in] m1 Reference matrix
  \param[in] m2 Matrix to align to the reference
  \param[in] apply if true, apply the transformation to m2 after alignment
  \param[in] pad set it to false if the matrices are already padded
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the cross correlation
**/
IMPEM2DEXPORT ResultAlign2D align2D_translational(algebra::Matrix2D_d &m1,
                                       algebra::Matrix2D_d &m2,
                                       bool apply=false);


//! Aligns two matrices rotationally and translationally without
//! performing preprocessing. Preprocessed data must be provided.
/**

  \param[in] m1 Reference matrix
  \param[in] m2 Matrix to align to the reference
  \param[in] M1 Fourier transform of m1
  \param[in] AUTOC_POLAR1 Fourier transform of the autocorrelation function
              of the reference matrix resampled to polar coordinates
  \param[in] AUTOC_POLAR2 Same thing form the matrix to align
  \param[in] apply if true, apply the transformation to m2 after alignment
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the cross correlation
**/
IMPEM2DEXPORT ResultAlign2D align2D_complete_no_preprocessing(
                      algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_c &M1,
                      algebra::Matrix2D_c &AUTOC_POLAR1,
                      algebra::Matrix2D_d &m2,
                      algebra::Matrix2D_c &AUTOC_POLAR2,
                      bool apply=false);



//! Aligns two matrices rotationally without performing preprocessing.
//! Preprocessed data must be provided.
/**
  \param[in] AUTOC_POLAR1 Fourier transform of the autocorrelation function
              of the reference matrix resampled to polar coordinates
  \param[in] AUTOC_POLAR2 Same thing form the matrix to align
  \param[in] n_rings number of rings used for the polar resamplings
  \param[in] sampling_points number of points used per ring for the
             polar resamplings
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the cross correlation
**/
IMPEM2DEXPORT ResultAlign2D align2D_rotational_no_preprocessing(
              algebra::Matrix2D_c &AUTOC_POLAR1,
              algebra::Matrix2D_c &AUTOC_POLAR2,
              unsigned int n_rings, unsigned int sampling_points);



//! Aligns two matrices translationally using the convolution theorem to
//! compute the cross correlation function.
/**
  \param[in] M1 FFT transform of the first matrix m1
  \param[in] M2 FFT transform of the second matrix m2
  \param[in] m1_rows Rows of the original matrix m1 employed to get M1
  \param[in] m1_cols Columns of the original matrix m1 employed to get M1
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the cross correlation
**/
IMPEM2DEXPORT ResultAlign2D align2D_translational_no_preprocessing(
               algebra::Matrix2D_c &M1,algebra::Matrix2D_c &M2,
               unsigned int m1_rows,unsigned int m1_cols);


//! Resamples a matrix to polar coordinates.
/**
  \param[in] m matrix to resample
  \param[out] result matrix to contain the resampling
  \param[in] interpolation_method interpolation method during resamplings,
              rotations and translations. Default:0, bilinear
**/
IMPEM2DEXPORT void resample2D_polar(algebra::Matrix2D_d &m,
                      algebra::Matrix2D_d &result,
                      int interpolation_method=0);

//! Performs a peak search in a Matrix2D
/**
  \param[in] m matrix
  \param[out] value the value at the peak
  \param[out] the position of the peak. Doubles, obained with interpolation
**/
IMPEM2DEXPORT algebra::Vector2D peak_search(
                                      algebra::Matrix2D_d &m,double *value);


//! Performs a peak search in a vector
/**
  \param[in] v vector
  \param[out] value the value at the peak
  \param[out] the position of the peak. Is a double, obtained with interpolation
**/
IMPEM2DEXPORT double peak_search(std::vector<double> &v,double *value);


//! computes the center of gravity (c.o.g.) for a subpart of the Matrix2D.
//! The weights for getting the c.o.g are the matrix values
/**
 \param[in] m Matrix
 \param[in] dims sizes for the subpart. Default is taking the entire matrix
 \param[in] bases starting indices for the subpart. Default is
            using the beginning of the matrix.
 \param[out] Vector2D containing the center of gravity
**/
IMPEM2DEXPORT algebra::Vector2D compute_center_of_gravity(
                   algebra::Matrix2D_d &m,int *dims=NULL,int *bases=NULL);

IMPEM2DEXPORT void print_vector(std::vector< std::complex<double> > &v);



//! Aligns two matrices (rotationally and translationally) using centers of
//! gravity
/**
  \param[in] m1 Reference matrix
  \param[in] m2 the matrix that is aligned with m1
  \param[in] apply if true, apply the transformation to m2 after alignment
  \param[in] interpolation_method interpolation method during resamplings,
              rotations and translations. Default:0, bilinear
  \param[out] A pair with the transformation to align m2 to m1
              and the value of the cross correlation
**/
IMPEM2DEXPORT ResultAlign2D align2D_complete_with_centers(
                      algebra::Matrix2D_d &m1,
                      algebra::Matrix2D_d &m2,
                      bool apply,
                      int interpolation_method=0);



//! Aligns two matrices (rotation and translation) using centers and
//! no preprocessing. Preprocessed data must be provided.
/**
  \param[in] center1 Center of gravity of the reference matrix
  \param[in] center2 Center of gravity of the matrix to align
  \param[in] AUTOC_POLAR1 Fourier transform of the autocorrelation function
              of the reference matrix resampled to polar coordinates
  \param[in] AUTOC_POLAR2 Same thing form the matrix to align
  \param[in] n_rings number of rings used for the polar resamplings
  \param[in] sampling_points number of points used per ring for the
             polar resamplings
  \param[out] A pair with the transformation to align the matrix
              to the reference and the value of the rotational
              cross correlation (NOT the cross-correlation coefficient)
**/
IMPEM2DEXPORT ResultAlign2D align2D_complete_with_centers_no_preprocessing(
                      algebra::Vector2D &center1,
                      algebra::Vector2D &center2,
                      algebra::Matrix2D_c &AUTOC_POLAR1,
                      algebra::Matrix2D_c &AUTOC_POLAR2,
                      unsigned int n_rings, unsigned int sampling_points);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_ALIGN_2D_H */
