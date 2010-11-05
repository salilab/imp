/**
 *  \file opencv_interface.h
 *  \brief inteface with OpenCV
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/opencv_interface.h"
#include "IMP/log.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE


// Normalize a openCV matrix to mean 0 and stddev 1. It is done in place
void normalize(cv::Mat &m) {
  cv::Scalar mean,stddev;
  cv::meanStdDev(m,mean,stddev);
  IMP_LOG(IMP::VERBOSE, "Matrix of mean: " << mean[0] << " stddev "
                  << stddev[0] << " normalized. " << std::endl);
  m = m - mean[0];
  m = m / stddev[0];
}


void show(cv::Mat &m,std::ostream &out) {
  for ( int i=0;i<m.rows;++i) {
    for ( int j=0;j<m.cols;++j) {
      out << m.at<double>(i,j) << " ";
    }
    out << std::endl;
  }
  out << std::endl;
}


IMPEM2D_END_NAMESPACE
