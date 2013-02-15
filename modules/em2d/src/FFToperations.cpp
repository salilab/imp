/**
 *  \file FFToperations.cpp
 *  \brief operations involving FFT
 Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/FFToperations.h"
#include "IMP/log.h"
#include "IMP/Pointer.h"

IMPEM2D_BEGIN_NAMESPACE


void get_autocorrelation2d_no_preprocessing(const cv::Mat &M, cv::Mat &corr) {
  IMP_LOG_VERBOSE(
              "Computing 2D autocorrelation no preprocessing" <<std::endl);
  IMP_USAGE_CHECK(((M.rows!=0) && (M.cols !=0)),
     "em2d:get_autocorrelation2d: Output matrix is empty");
  cv::Mat temp;
  cv::mulSpectrums(M,M, temp,0,true);
  cv::idft(temp, temp,cv::DFT_SCALE+cv::DFT_REAL_OUTPUT);
  temp(cv::Rect(0, 0, corr.cols, corr.rows)).copyTo(corr);
  do_matrix_to_image_flip(corr);
}


void get_autocorrelation2d(const cv::Mat &m, cv::Mat &corr) {
  IMP_LOG_VERBOSE("Computing 2D autocorrelation " <<std::endl);
  // resize the output array if needed
  cv::Size dftSize;
  // compute the optimal size for faster DFT transform
  dftSize.width = cv::getOptimalDFTSize(m.cols);
  dftSize.height = cv::getOptimalDFTSize(m.rows);
  // temporary
  cv::Mat tempA(dftSize, m.type(), cv::Scalar::all(0));
  // copy A to the top-left corner of tempA
  cv::Mat roiA(tempA, cv::Rect(0,0,m.cols,m.rows));
  m.copyTo(roiA);
  cv::dft(tempA, tempA,0,m.rows);
  cv::Mat temp;
  cv::mulSpectrums(tempA,tempA, temp,0,true);

  cv::idft(temp, temp,cv::DFT_SCALE+cv::DFT_REAL_OUTPUT);
  corr.create(m.rows,m.cols,m.type());
  temp(cv::Rect(0, 0, corr.cols, corr.rows)).copyTo(corr);
  em2d::do_matrix_to_image_flip(corr);

}




void get_correlation2d(const cv::Mat &A, const cv::Mat &B, cv::Mat &corr) {

  IMP_LOG_VERBOSE("Computing 2D correlation " <<std::endl);

  IMP_USAGE_CHECK(((A.rows==B.rows) && (A.cols == B.cols)),
                  "em2d:get_correlation2d: Matrices have different size.");
  // resize the output array if needed
  corr.create(A.rows,A.cols, A.type());
  cv::Size dftSize;
  // compute the optimal size for faster DFT transform
  dftSize.width = cv::getOptimalDFTSize(A.cols);
  dftSize.height = cv::getOptimalDFTSize(A.rows);

  // temporary matrices
  cv::Mat tempA(dftSize, A.type(), cv::Scalar::all(0));
  cv::Mat tempB(dftSize, B.type(), cv::Scalar::all(0));

  // copy A and B to the top-left corners of tempA and tempB, respectively
  cv::Mat roiA(tempA, cv::Rect(0,0,A.cols,A.rows));
  A.copyTo(roiA);
  cv::Mat roiB(tempB, cv::Rect(0,0,B.cols,B.rows));
  B.copyTo(roiB);

  // FFT the padded A & B in-place;
  // use "nonzeroRows" hint for faster processing
  cv::dft(tempA, tempA, 0, A.rows);
  cv::dft(tempB, tempB, 0, B.rows);

  // multiply the spectrums;
  // the function handles packed spectrum representations well
  cv::mulSpectrums(tempA, tempB, tempA,0,true);

  // inverse transform
  // Even though all the result rows will be non-zero,
  // we need only the first corr.rows of them, and thus we
  // pass nonzeroRows == corr.rows
  cv::dft(tempA, tempA, cv::DFT_INVERSE + cv::DFT_SCALE, corr.rows);

  // now copy the result back to C.
  tempA(cv::Rect(0, 0, corr.cols, corr.rows)).copyTo(corr);
  do_matrix_to_image_flip(corr);
}



void get_correlation2d_no_preprocessing(const cv::Mat &M1,
                                    const cv::Mat &M2, cv::Mat &corr) {

  IMP_LOG_VERBOSE("Computing 2D correlation no preprocessing "<<std::endl);

  IMP_USAGE_CHECK(((M1.rows==M2.rows) && (M1.cols == M2.cols)),
                  "em2d:get_correlation2d: Matrices have different size.");

  cv::Mat temp;
  cv::mulSpectrums(M1, M2, temp,0,true);
  cv::idft(temp, temp,cv::DFT_SCALE+cv::DFT_REAL_OUTPUT);
  // now copy the result to corr
  temp(cv::Rect(0, 0, corr.cols, corr.rows)).copyTo(corr);
  do_matrix_to_image_flip(corr);
}

void get_fft_using_optimal_size(const cv::Mat &m,cv::Mat &M) {
  // get the optimal size
  cv::Size dftSize;
  dftSize.width = cv::getOptimalDFTSize(m.cols);
  dftSize.height = cv::getOptimalDFTSize(m.rows);
  // create a temporal matrix with the optimal size and copy m to the region
  cv::Mat temp(dftSize, m.type(), cv::Scalar::all(0));
  // copy A and B to the top-left corners of tempA and tempB, respectively
  cv::Mat region(temp, cv::Rect(0,0,m.cols,m.rows));
  m.copyTo(region);
  cv::dft(temp, temp, 0,m.rows); // quick fft given the optimal size
  // Copy to M
  temp(cv::Rect(0, 0, m.cols, m.rows)).copyTo(M);
}



void get_spectrum(const cv::Mat &m, cv::Mat &real,cv::Mat &imag) {
  cv::Size dftSize;
  // compute the optimal size for faster DFT transform
  dftSize.width = cv::getOptimalDFTSize(m.cols);
  dftSize.height = cv::getOptimalDFTSize(m.rows);
  // temporary

  cv::Mat real_temp(dftSize, m.type(), cv::Scalar::all(0));
  cv::Mat roiA(real_temp, cv::Rect(0,0,m.cols,m.rows));

  // Shift to get a centered spectrum
  for ( int i=0;i<m.rows;++i) {
    for ( int j=0;j<m.cols;++j) {
      if((i+j)%2 == 1) { // the sum is an odd number
        roiA.at<double>(i,j) = (-1) *m.at<double>(i,j);
      } else {
        roiA.at<double>(i,j) = m.at<double>(i,j);
      }
    }
  }

  cv::Mat imag_temp(dftSize,m.type(),cv::Scalar::all(0.0));
  // Merge and dft
  cv::Mat temp,TEMP;
  cv::Mat imgs[]= { real_temp,imag_temp };
  cv::merge(imgs,2,temp);
  cv::dft(temp, TEMP,cv::DFT_COMPLEX_OUTPUT,m.rows);
  // Real and imaginary
  // recover real part from the first channel
  // both source and destination matrices must be allocateda
  int rows = temp.rows;
  int cols=  temp.cols;
  cv::Mat TEMP_1st_channel(rows,cols,m.type()); // real
  cv::Mat TEMP_2nd_channel(rows,cols,m.type()); // imag
  cv::Mat output[] = { TEMP_1st_channel,TEMP_2nd_channel };
  const int fromTo[] = { 0,0 , 1,1 };// see mixChannels help, continuous numbers
  cv::mixChannels(&TEMP,1,output,2, fromTo, 2);
  // resize the output arrays if needed
  real.create(m.rows,m.cols, m.type());
  imag.create(m.rows,m.cols, m.type());
  TEMP_1st_channel(cv::Rect(0, 0, real.cols,real.rows)).copyTo(real);
  TEMP_2nd_channel(cv::Rect(0, 0, imag.cols,imag.rows)).copyTo(imag);
}


void do_matrix_to_image_flip(cv::Mat &m) {
  int half_rows = m.rows/2;
  int half_columns = m.cols/2;
  int new_i,new_j;
  for (int i=0;i<m.rows;++i) {
    for (int j=0;j<half_columns;++j ) {
      if(i>=half_rows) { new_i=i-half_rows; } else { new_i=i+half_rows; }
      new_j=j+half_columns;
      std::swap(m.at<double>(i,j),m.at<double>(new_i,new_j));
    }
  }
}


IMPEM2D_END_NAMESPACE
