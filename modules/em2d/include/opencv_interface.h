/**
 *  \file opencv_interface.h
 *  \brief inteface with OpenCV
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_OPENCV_INTERFACE_H
#define IMPEM2D_OPENCV_INTERFACE_H

#include "IMP/em2d/em2d_config.h"
#include "opencv/cv.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

typedef cv::MatIterator_<double> CVDoubleMatIterator;


// Normalize a openCV matrix to mean 0 and stddev 1. It is done in place
IMPEM2DEXPORT void normalize(cv::Mat &m);

IMPEM2DEXPORT void show(cv::Mat &m,std::ostream &out = std::cout);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_OPENCV_INTERFACE_H */
