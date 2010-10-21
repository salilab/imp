/**
 *  \file FFToperations.h
 *  \brief Operations involving FFT
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_FF_TOPERATIONS_H
#define IMPEM2D_FF_TOPERATIONS_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/em2d/FFTinterface.h"
#include "IMP/em2d/opencv_interface.h"
#include "IMP/algebra/Matrix2D.h"

IMPEM2D_BEGIN_NAMESPACE

//! Returns the correlation matrix between two 2D matrices using FFT
/**
  \param[in] m1 first matrix
  \param[in] m2 second matrix
  \param[out] corr matrix of results
**/
IMPEM2DEXPORT void correlation2D(algebra::Matrix2D_d &m1,
                          algebra::Matrix2D_d &m2,
                          algebra::Matrix2D_d &corr);


//! Returns the correlation matrix between two 2D matrices using FFT
/**
  \param[in] M1 FFT transform of the first matrix
  \param[in] M2 FFT transform of the second matrix
  \param[out] corr matrix of results (IT MUST have the right size in advance)
**/
IMPEM2DEXPORT void correlation2D_no_preprocessing(
                   algebra::Matrix2D_c &M1,
                   algebra::Matrix2D_c &M2,
                   algebra::Matrix2D_d &corr);

//! Returns the autocorrelation matrix using FFT
/**
  \param[in] m matrix to autocorrelate
  \param[out] corr matrix of results
**/
IMPEM2DEXPORT void autocorrelation2D(algebra::Matrix2D_d &m,
                               algebra::Matrix2D_d &corr);

//! Returns the autocorrelation matrix using FFT
/**
  \param[in] M FFT transform of the matrix to autocorrelate
  \param[out] corr matrix of results (IT MUST have the right size in advance)
**/
IMPEM2DEXPORT void autocorrelation2D_no_preprocessing(
                               algebra::Matrix2D_c &M,
                               algebra::Matrix2D_d &corr);



//! Help functions to print complex matrices (for debugging purposes)

IMPEM2DEXPORT void  print_matrix(algebra::Matrix2D_c &m,
                                std::ostream &out = std::cout);

IMPEM2DEXPORT void print_matrix(algebra::Matrix3D<std::complex<double> > &m,
                              std::ostream &out = std::cout);

IMPEM2DEXPORT void Matrix2D_c_to_img(algebra::Matrix2D_c &M,String name);



//! Transforms a Matrix3D as is given by FFT functions, into a volume
//! interpretation. Works the opposite way (volume to Matrix3D) too
template<typename T>
void matrix_to_volume_interpretation(algebra::Matrix3D<T> &m) {
  int half_slices = m.get_number_of_slices()/2;
  int half_rows   = m.get_number_of_rows()/2;
  int half_columns= m.get_number_of_columns()/2;
  int new_i,new_j,new_k;
  for (int i=0;i<m.get_number_of_slices();++i ) {
    for (int j=0;j<m.get_number_of_rows();++j ) {
      for (int k=0;k<half_columns;++k ) {
        if(i>=half_slices) {new_i=i-half_slices;} else {new_i=i+half_slices;}
        if(j>=half_rows)   {new_j=j-half_rows;}   else {new_j=j+half_rows;}
        new_k=k+half_columns;
        std::swap(m(i,j,k),m(new_i,new_j,new_k));
      }
    }
  }
}

//! Transforms a Matrix2D as is given by FFT functions, into a image
//! interpretation. Works the opposite way (volume to Matrix3D) too
template<typename T>
void matrix_to_image_interpretation(algebra::Matrix2D<T> &m) {
  int half_rows = m.get_number_of_rows()/2;
  int half_columns = m.get_number_of_columns()/2;
  int new_i,new_j;
  for (int i=0;i<m.get_number_of_rows();i++ ) {
    for (int j=0;j<half_columns;j++ ) {
      if(i>=half_rows) { new_i=i-half_rows; } else { new_i=i+half_rows; }
      new_j=j+half_columns;
      std::swap(m(i,j),m(new_i,new_j));
    }
  }
}


//! Transforms a matrix as is given by FFT functions, into a image
//! interpretation. Works the opposite way.
IMPEM2DEXPORT void matrix_to_image_flip(cv::Mat &m);

////! Returns the correlation matrix between two 2D matrices using FFT
///
//  \param[in] A first matrix
//  \param[in] B second matrix
//  \param[out] corr matrix of results
//**/
//IMPEM2DEXPORT void correlation2D(const cv::Mat& A, const cv::Mat& B,
//                                 cv::Mat& corr);
//

//! Computes the autocorrelation without preprocessing
/**
  \param[in] M Matrix containing a Fourier Transform (it is not checked that
               the matrix actually contains the trasnform)
  \param[out] corr matrix of results
**/
IMPEM2DEXPORT void autocorrelation2D_no_preprocessing(
                                      const cv::Mat& M, cv::Mat& corr);

//! Returns the autocorrelation matrix
/**
  \param[in] m first matrix
  \param[out] corr matrix of results
**/
IMPEM2DEXPORT void autocorrelation2D(const cv::Mat& m, cv::Mat& corr);

//! Correlation matrix between two 2D matrices using FFT
/**
  \param[in] A first matrix
  \param[in] B second matrix
  \param[out] corr matrix of results
**/
IMPEM2DEXPORT void correlation2D(const cv::Mat& A,
                                 const cv::Mat& B, cv::Mat& corr);


//! Returns the correlation matrix between two 2D matrices using FFT
/**
  \param[in] M1 FFT transform of the first matrix
  \param[in] M2 FFT transform of the second matrix
  \param[out] corr matrix of results (It MUST have the right size in advance)
**/
IMPEM2DEXPORT void correlation2D_no_preprocessing(const cv::Mat& M1,
                                    const cv::Mat& M2, cv::Mat& corr);




IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_FF_TOPERATIONS_H */
