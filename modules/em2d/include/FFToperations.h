/*!
 *  \file FFToperations.h
 *  \brief Operations involving FFT
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FF_TOPERATIONS_H
#define IMPEM2D_FF_TOPERATIONS_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"

IMPEM2D_BEGIN_NAMESPACE


//! Transforms a matrix as is given by FFT functions, into a image
//! interpretation. Works the opposite way.
IMPEM2DEXPORT void do_matrix_to_image_flip(cv::Mat &m);




//! Autocorrelation without preprocessing
/*!
  \param[in]  M matrix containing the dft
   \param[out] corr the matrix to store the autocorrelation. Must have the
    proper dimensions when passed
*/
IMPEM2DEXPORT void get_autocorrelation2d_no_preprocessing(
                                      const cv::Mat &M, cv::Mat &corr);

//! Returns the autocorrelation matrix
/*!
  \param[in] m first matrix
  \param[out] corr matrix of results
*/
IMPEM2DEXPORT void get_autocorrelation2d(const cv::Mat &m, cv::Mat &corr);


//! Correlation matrix between two 2D matrices using FFT
/*!
  \param[in] A first matrix
  \param[in] B second matrix
  \param[out] corr matrix of results
*/
IMPEM2DEXPORT void get_correlation2d(const cv::Mat &A,
                                 const cv::Mat &B, cv::Mat &corr);


//! Correlation without preprocessing
//! Returns the correlation matrix between two 2D matrices using FFT
/*!
  \param[in]  M1 matrix containing the dft of the first matrix
  \param[in]  M2 matrix containing the dft of the second matrix
   \param[out] corr matrix of results (It MUST have the right size in advance)
*/
IMPEM2DEXPORT void get_correlation2d_no_preprocessing(const cv::Mat &M1,
                                    const cv::Mat &M2, cv::Mat &corr);


//! Get the FFT of a matrix using padding with other matrix that can be
//! computed with optimized FFT
IMPEM2DEXPORT void get_fft_using_optimal_size(const cv::Mat &m,cv::Mat &M);

//! Computes the fft of a matrix and returns the real and imaginary matrices
IMPEM2DEXPORT void get_spectrum(const cv::Mat &m, cv::Mat &real,cv::Mat &imag);


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_FF_TOPERATIONS_H */
