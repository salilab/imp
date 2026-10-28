/**
 *  \file Matrix.cpp
 *  \brief Matrices used to store image data
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/Matrix.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/Image.h"
#include "IMP/Pointer.h"
#include "IMP/log.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE

void show(const DoubleMatrix &m, std::ostream &out) {
  for (int i = 0; i < m.rows(); ++i) {
    for (int j = 0; j < m.cols(); ++j) {
      out << m(i, j) << " ";
    }
    out << std::endl;
  }
  out << std::endl;
}

void write_matrix(const DoubleMatrix &m, std::string name) {
  IMP_NEW(em2d::SpiderImageReaderWriter, srw, ());
  IMP_NEW(em2d::Image, output_img, ());
  output_img->set_was_used(true);
  output_img->set_data(m);
  output_img->write(name, srw);
}

IMPEM2D_END_NAMESPACE
