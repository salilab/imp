/*!
 *  \file Image.cpp
 *  \brief Management of IMP images for Electron Microscopy
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/Image.h"
#include "IMP/em2d/scores2D.h"

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



void save_images(Images images, Strings names,
                      const em2d::ImageReaderWriter<double> &rw) {
  for(unsigned long i=0;i<images.size();++i) {
    images[i]->write_to_floats(names[i],rw);
  }
}



double cross_correlation_coefficient(em2d::Image *im1,
                                     em2d::Image *im2) {
  return cross_correlation_coefficient(im1->get_data(),im2->get_data());
}

void normalize(em2d::Image *im,bool force) {
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


void resample_polar(em2d::Image *im1,em2d::Image *im2,
                const PolarResamplingParameters &polar_params) {
  resample_polar(im1->get_data(),im2->get_data(),polar_params);
}

void add_noise(em2d::Image *im1,double op1, double op2,
                                    const String &mode, double df) {
  add_noise(im1->get_data(),op1,op2,mode,df);
}

IMPEM2D_END_NAMESPACE
