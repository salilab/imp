/**
 *  \file opencv_interface.h
 *  \brief inteface with OpenCV
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/opencv_interface.h"
#include "IMP/em2d/SpiderImageReaderWriter.h"
#include "IMP/em2d/Image.h"
#include "IMP/base/Pointer.h"
#include "IMP/base/log.h"
#include "IMP/macros.h"

IMPEM2D_BEGIN_NAMESPACE


void show(const cv::Mat &m,std::ostream &out) {
  for ( int i=0;i<m.rows;++i) {
    for ( int j=0;j<m.cols;++j) {
      out << m.at<double>(i,j) << " ";
    }
    out << std::endl;
  }
  out << std::endl;
}


void write_matrix(cv::Mat &m,std::string name) {
  IMP_NEW(em2d::SpiderImageReaderWriter, srw, ());
  IMP_NEW(em2d::Image,output_img,());
  output_img->set_was_used(true);
  output_img->set_data(m);
  output_img->write(name,srw);
}



IMPEM2D_END_NAMESPACE
