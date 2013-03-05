/**
 *  \file opencv_interface.h
 *  \brief inteface with OpenCV
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_OPENCV_INTERFACE_H
#define IMPEM2D_OPENCV_INTERFACE_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/algebra/Transformation2D.h"

#if IMP_EM2D_HAS_OPENCV22
#include "opencv2/core/core.hpp"
#include "opencv2/core/version.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#else
#include "opencv/cv.h"
#include "opencv/highgui.h"
#endif

#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

typedef cv::Mat_<double> cvDoubleMat;
typedef cv::MatIterator_<double> cvDoubleMatIterator;
typedef cv::MatConstIterator_<double> cvDoubleConstMatIterator;

typedef cv::Mat_<int> cvIntMat;
typedef cv::MatIterator_<int> cvIntMatIterator;

typedef cv::Point_<int> cvPixel;
typedef std::vector< cvPixel > cvPixels;


//! Prints a OpenCV matrix
IMPEM2DEXPORT void show(const cv::Mat &m,std::ostream &out = std::cout);

//! Quick and dirty way of writing a OpenCV matrix to a Spider image
IMPEM2DEXPORT void write_matrix(cv::Mat &m,std::string name);


//! Show a Mat_
template<typename T>
void show(const cv::Mat_<T> &m,std::ostream &out = std::cout) {
  for ( int i=0;i<m.rows;++i) {
    for ( int j=0;j<m.cols;++j) {
      out << m(i,j) << " ";
    }
    out << std::endl;
  }
  out << std::endl;
}


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_OPENCV_INTERFACE_H */
