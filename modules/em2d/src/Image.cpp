/*!
 *  \file Image.cpp
 *  \brief Management of IMP images for Electron Microscopy
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#include "IMP/em2d/Image.h"
#include "IMP/em2d/scores2D.h"
#include "IMP/Pointer.h"

IMPEM2D_BEGIN_NAMESPACE

Image::Image() {
  name_ = "";
  header_.set_image_type(em::ImageHeader::IMG_IMPEM);
  // Stats not computed
  header_.set_fSig(-1);
  header_.set_fImami(0);
}


//! Constructor with size
Image::Image(int rows, int cols) {
  resize_data(rows,cols);
  header_.set_header();
  header_.set_image_type(em::ImageHeader::IMG_IMPEM);
  header_.set_fSig(-1);
  header_.set_fImami(0);
}


void Image::resize(int rows,int cols) {
  resize_data(rows,cols);
  header_.set_number_of_slices(1.0);
  header_.set_number_of_rows(rows);
  header_.set_number_of_columns(cols);
}

void Image::resize_data(int rows,int cols) {
  data_.create(rows,cols,CV_64FC1);
}

void Image::resize(Image *img) {
  resize(img->get_data().rows,img->get_data().cols);
}


void Image::set_data(const cv::Mat &mat) {
  mat.copyTo(data_);
  adjust_header();
}

void Image::adjust_header() {
    header_.set_image_type(em::ImageHeader::IMG_IMPEM);
    header_.set_number_of_slices(1.0);
    header_.set_number_of_rows(data_.rows);
    header_.set_number_of_columns(data_.cols);
    header_.set_header(); // Initialize header
  }




Images read_images(const Strings &names,
                   const em2d::ImageReaderWriter<double> &rw) {
  unsigned long size = names.size();
  Images v(size);
  for(unsigned long i=0;i<size;++i) {
    v[i]=new Image(names[i],rw);
  }
  return v;
}


void save_images(Images images,const Strings &names,
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

void subtract_images(em2d::Image *first,em2d::Image *second,
                                  em2d::Image *result) {
  cv::Mat result_matrix;
  cv::subtract(first->get_data(),
               second->get_data(),
               result_matrix);
  result->set_data(result_matrix);
}

IMPEM2D_END_NAMESPACE
