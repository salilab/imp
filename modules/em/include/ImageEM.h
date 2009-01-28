/**
 *  \file ImageEM.h
 *  \brief Management of IMP images for Electron Microscopy
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_IMAGE_EM_H
#define IMPEM_IMAGE_EM_H

#include <limits>
#include <typeinfo>
#include "config.h"
#include "ImageHeader.h"
#include "IMP/algebra/Matrix2D.h"

IMPEM_BEGIN_NAMESPACE


//! Template class for managing 2D Electron Microscopy images in IMP
template<typename T>
class IMPEMEXPORT ImageEM
{
public:
  //! Empty constructor
  ImageEM() {
    name_ = "";
    if (typeid(T) == typeid(Float)) {
      header_.set_image_type(ImageHeader::IMG_IMPEM);
    }
  }

  //! Constructor with size
  ImageEM(Int Ydim, Int Xdim) {
    data_.resize(Ydim, Xdim);
    header_.set_header();
    if (typeid(T) == typeid(Float)) {
      header_.set_image_type(ImageHeader::IMG_IMPEM);
    }
  }

  //! Access to the matrix of data
  algebra::Matrix2D<T>& get_data() {
    return data_;
  }

  //! Access to the header
  ImageHeader& get_header() {
    return header_;
  }

  //! Adjusts the information of the imager header taking into account the
  //! dimensions of the data and setting the time, date, type, etc ...
  void adjust_header() {
    if (typeid(T) == typeid(std::complex< double >)) {
      header_.set_image_type(ImageHeader::IMG_FOURIER);
    } else if (typeid(T) == typeid(double) ||
               typeid(T) == typeid(Float)) {
      header_.set_image_type(ImageHeader::IMG_IMPEM);
    }
    header_.set_slices(1.0);
    header_.set_rows(data_.get_rows());
    header_.set_columns(data_.get_columns());
    header_.set_time();
    header_.set_date();
    header_.set_title(name_);
    header_.set_header(); // Initialize header
  }

  //! Reads the image from the file
  void read(String filename) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    header_.read(in);
    // Adjust size of the matrix according to the header
    data_.resize(header_.get_rows(), header_.get_columns());
    data_.read_binary(in);
    in.close();
  }

  //! Writes the image to a file
  void write(String filename, bool force_reversed = false) {
    adjust_header(); // First adjust the header to guarantee consistence
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    header_.write(out, force_reversed);
    data_.write_binary(out);
    out.close();
  }

protected:
  //! Name of the image. Frequently it will be the name of the file
  String name_;
  //! Matrix with the data for the image
  algebra::Matrix2D<T> data_;
  //! Header for the image with all the pertinent information
  ImageHeader header_;

}; // ImageEM

IMPEM_END_NAMESPACE
#endif  /* IMPEM_IMAGE_EM_H */
