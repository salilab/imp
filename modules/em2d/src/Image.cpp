/**
 *  \file Image.cpp
 *  \brief Management of IMP images for Electron Microscopy
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
**/

#include "IMP/em2d/Image.h"

IMPEM2D_BEGIN_NAMESPACE

Images read_images(Strings names,const em2d::ImageReaderWriter<double> &rw) {
  unsigned long size = names.size();
  Images v(size);
  for(unsigned long i=0;i<size;++i) {
    v[i]= new Image();
    v[i]->read_from_floats(names[i],rw);
  }
  return v;
}


void save_images(Images images, Strings names, const
                      em2d::ImageReaderWriter<double> &rw) {
  for(unsigned long i=0;i<images.size();++i) {
    images[i]->write_to_floats(names[i],rw);
  }
}


void subtract_images(em2d::Image *first,em2d::Image *second,
                                  em2d::Image *result) {

  cv::Mat result_matrix;
  cv::subtract(first->get_data(),
               second->get_data(),
               result_matrix);
  result->set_data(result_matrix);
}


void normalize(Image *im,bool force) {
  if(!im->get_header().is_normalized() || force==true ) {
    normalize(im->get_data());
    im->get_header().set_fImami(1);
    im->get_header().set_fAv(0.0);
    im->get_header().set_fSig(1.0);
    double minVal,maxVal;
    cv::minMaxLoc(im->get_data(),&minVal,&maxVal);
    im->get_header().set_fFmin(minVal);
    im->get_header().set_fFmax(maxVal);
  }
}


IMPEM2D_END_NAMESPACE
