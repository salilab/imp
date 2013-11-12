/*!
 *  \file FFToperations.h
 *  \brief Operations involving FFT
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FF_TOPERATIONS_H
#define IMPEM2D_FF_TOPERATIONS_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/opencv_interface.h"

IMPEM2D_BEGIN_NAMESPACE

/**
 * Transforms a matrix as is given by FFT functions, into a image
 * interpretation. Works the opposite way too.
 * @param m The matrix to flip. The it is changed in situ
 */
IMPEM2DEXPORT void do_matrix_to_image_flip(cv::Mat &m);

//! Autocorrelation without preprocessing
/*!
  \param[in]  M matrix containing the dft
   \param[out] corr the matrix to store the autocorrelation. Must have the
    proper dimensions when passed
*/
/**
 * Autocorrelation without preprocessing.
 * @param  M matrix containing the dft
 * @param corr The matrix to store the autocorrelation. Must have the
 * proper dimensions when passed.
 */
IMPEM2DEXPORT void get_autocorrelation2d_no_preprocessing(const cv::Mat &M,
                                                          cv::Mat &corr);

/**
 * Computes the autocorrelation matrix
 * @param m The input matrix
 * @param corr The result matrix containing the autocorrelation
 */
IMPEM2DEXPORT void get_autocorrelation2d(const cv::Mat &m, cv::Mat &corr);

//! Correlation matrix between two 2D matrices using FFT
/*!
  \param[in] A first matrix
  \param[in] B second matrix
  \param[out] corr matrix of results
*/
IMPEM2DEXPORT void get_correlation2d(const cv::Mat &A, const cv::Mat &B,
                                     cv::Mat &corr);

//! Correlation without preprocessing
//! Returns the correlation matrix between two 2D matrices using FFT
/*!
  \param[in]  M1 matrix containing the dft of the first matrix
  \param[in]  M2 matrix containing the dft of the second matrix
   \param[out] corr matrix of results (It MUST have the right size in advance)
*/
IMPEM2DEXPORT void get_correlation2d_no_preprocessing(const cv::Mat &M1,
                                                      const cv::Mat &M2,
                                                      cv::Mat &corr);

/**
 * Get the FFT of a matrix using padding with other matrix that can be
 * computed with FFT in an optimal way. i.e. with a size that makes the
 * FFT algorithm work faster
 * @param m The input matrix
 * @param M The output matrix with the FFT
 * @note The output matrix can have (and frequently will have) different
 * dimensions than the input matrix
 */
IMPEM2DEXPORT void get_fft_using_optimal_size(const cv::Mat &m, cv::Mat &M);

/**
 * Computes the fft of a matrix and returns the real and imaginary matrices
 * @param m The input matrix
 * @param real The matrix with the real part of the FFT matrix
 * @param imag The imaginary part of the FFT matrix
 */
IMPEM2DEXPORT void get_spectrum(const cv::Mat &m, cv::Mat &real, cv::Mat &imag);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_FF_TOPERATIONS_H */
