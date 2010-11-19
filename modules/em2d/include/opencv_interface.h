/**
 *  \file opencv_interface.h
 *  \brief inteface with OpenCV
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_OPENCV_INTERFACE_H
#define IMPEM2D_OPENCV_INTERFACE_H

#include "IMP/em2d/em2d_config.h"
#include "IMP/algebra/Transformation2D.h"
#include "opencv/cv.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

typedef cv::MatIterator_<double> CVDoubleMatIterator;

//! Prints a OpenCV matrix
IMPEM2DEXPORT void show(cv::Mat &m,std::ostream &out = std::cout);

//! Quick and dirty way of writing a OpenCV matrix to a Spider image
IMPEM2DEXPORT void write_matrix(cv::Mat &m,std::string name);


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_OPENCV_INTERFACE_H */
