/**
 *  \file Image.h
 *  \brief Management of IMP images for Electron Microscopy
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_IMAGE_H
#define IMPEM_IMAGE_H

// #define DEBUG

#include "config.h"
#include "ImageHeader.h"
#include "ImageReaderWriter.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Rotation2D.h"
#include <complex>
#include <limits>
#include <typeinfo>

IMPEM_BEGIN_NAMESPACE


//! Template class for managing 2D Electron Microscopy images in IMP
template<typename T>
class Image
{
public:

typedef Image<T> This;

  //! Empty constructor
  Image() {
    name_ = "";
    if (typeid(T) == typeid(Float)) {
      header_.set_image_type(ImageHeader::IMG_IMPEM);
    }
  }

  //! Constructor with size
  Image(Int Ydim, Int Xdim) {
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
  void read(String filename,ImageReaderWriter<T>& reader) {
#ifdef DEBUG
    std::cout << "reading EM image " << std::endl;
#endif
    reader.read(filename,header_,data_);
  }

  //! Writes the image to a file
  void write(String filename, ImageReaderWriter<T>& writer) {
    adjust_header(); // First adjust the header to guarantee consistence
    writer.write(filename,header_,data_);
  }


  //! Rotates the image a given angle in radians. The center is the center of
  //! rotation.
  /**
    \param[in] ang angle
    \param[in] wrap if true, the Matrix2D is wrapped.
  **/
  void rotate(double ang,bool wrap=false) {
    IMP::algebra::auto_rotate_matrix_2D(this->get_data(),ang,wrap);
  }

  //! The image is used as reference to translationally align the
  //! parameter image with it.
  /**
    \param[in] img Image to align with the reference
    \todo Under implementation
  **/
  void align2d_trans(This& img) {
    int a = 1;
  }

  //! The image is used as reference to rotationally align the
  //! parameter image with it.
  /**
    \param[in] img Image to align with the reference
    \todo Under implementation
  **/
  void align2d_rot(This& img) {
    int a = 1;
  }

  //! The image is used as reference to rotationally and translationally align
  //! the parameter image with it.
  /**
    \param[in] img Image to align with the reference
    \todo Under implementation
  **/
  void align2d_complete(This& img) {
    int a = 1;
  }

protected:
  //! Name of the image. Frequently it will be the name of the file
  String name_;
  //! Matrix with the data for the image
  algebra::Matrix2D<T> data_;
  //! Header for the image with all the pertinent information
  ImageHeader header_;

}; // Image

// #undef DEBUG

IMPEM_END_NAMESPACE
#endif  /* IMPEM_IMAGE_H */
