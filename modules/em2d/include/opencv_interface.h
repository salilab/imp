/**
 *  \file IMP/em2d/opencv_interface.h
 *  \brief Interface with OpenCV
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_OPENCV_INTERFACE_H
#define IMPEM2D_OPENCV_INTERFACE_H

#include <IMP/em2d/em2d_config.h>
#include "IMP/algebra/Transformation2D.h"

#if IMP_EM2D_HAS_OPENCV22 || IMP_EM2D_HAS_OPENCV3
#include "opencv2/core/core.hpp"
#include "opencv2/core/version.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#else
#include "opencv/cv.h"
#include "opencv/highgui.h"
#endif

#include <iostream>
#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

IMPEM2D_BEGIN_NAMESPACE

typedef cv::Mat_<double> cvDoubleMat;
typedef cv::MatIterator_<double> cvDoubleMatIterator;
typedef cv::MatConstIterator_<double> cvDoubleConstMatIterator;

typedef cv::Mat_<int> cvIntMat;
typedef cv::MatIterator_<int> cvIntMatIterator;

typedef cv::Point_<int> cvPixel;
typedef std::vector<cvPixel> cvPixels;

//! Prints a OpenCV matrix
IMPEM2DEXPORT void show(const cv::Mat &m, std::ostream &out = std::cout);

//! Quick and dirty way of writing a OpenCV matrix to a Spider image
IMPEM2DEXPORT void write_matrix(cv::Mat &m, std::string name);

//! Show a Mat_
template <typename T>
void show(const cv::Mat_<T> &m, std::ostream &out = std::cout) {
  for (int i = 0; i < m.rows; ++i) {
    for (int j = 0; j < m.cols; ++j) {
      out << m(i, j) << " ";
    }
    out << std::endl;
  }
  out << std::endl;
}

IMPEM2D_END_NAMESPACE

namespace cereal {
  template<class Archive>
  inline void serialize(Archive &ar, cv::Mat &m) {
    int rows, cols, type;
    bool continuous;

    if (std::is_base_of<cereal::detail::OutputArchiveBase, Archive>::value) {
      rows = m.rows;
      cols = m.cols;
      type = m.type();
      continuous = m.isContinuous();
    }
    ar(rows, cols, type, continuous);

    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      m.create(rows, cols, type);
    }

    if (continuous) {
      size_t data_size = rows * cols * m.elemSize();
      auto mat_data = cereal::binary_data(m.data, data_size);
      ar(mat_data);
    } else {
      size_t row_size = cols * m.elemSize();
      for (int i = 0; i < rows; ++i) {
        auto row_data = cereal::binary_data(m.ptr(i), row_size);
        ar(row_data);
      }
    }
  }
}

#endif /* IMPEM2D_OPENCV_INTERFACE_H */
