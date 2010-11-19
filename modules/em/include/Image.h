/**
 *  \file em/Image.h
 *  \brief Management of IMP images for Electron Microscopy
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM_IMAGE_H
#define IMPEM_IMAGE_H

#include "IMP/em/em_config.h"
#include "IMP/em/ImageHeader.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/Rotation2D.h"
#include "IMP/em/ImageReaderWriter.h"
#include <IMP/Object.h>
#include "IMP/VectorOfRefCounted.h"
#include <complex>
#include <limits>
#include <typeinfo>

IMPEM_BEGIN_NAMESPACE

//! 2D Electron Microscopy images in IMP
class IMPEMEXPORT Image : public Object
{
public:

  //! Empty constructor
  Image() {
    name_ = "";
    header_.set_image_type(ImageHeader::IMG_IMPEM);
    // Stats not computed
    header_.set_fSig(-1);
    header_.set_fImami(0);
  }

  //! Constructor with size
  Image(Int Ydim, Int Xdim) {
    data_.resize(Ydim, Xdim);
    header_.set_header();
    header_.set_image_type(ImageHeader::IMG_IMPEM);
    header_.set_fSig(-1);
    header_.set_fImami(0);
  }

  //! Access to the matrix of data
  algebra::Matrix2D_d& get_data() {
    return data_;
  }

  //! Sets the entire matrix of data
  void set_data(algebra::Matrix2D_d &v) {
    data_.resize(v);
    for(unsigned long i=0;i<v.num_elements();i++) {
      data_.data()[i] = v.data()[i];
    }
    this->adjust_header();
  }


  //! Set the value of one LOGICAL element of the matrix of data
  void set_value(int i, int j,double val) const {
    data_(i,j)=val;
  }

  //! Access to one LOGICAL element of the matrix of data
  double operator()(int i, int j) const {
    return data_(i,j);
  }

  //! Access to the header
  inline ImageHeader& get_header() {
    return header_;
  }


  void resize(int rows,int cols) {
    data_.resize(rows,cols);
    header_.set_number_of_slices(1.0);
    header_.set_number_of_rows(rows);
    header_.set_number_of_columns(cols);
  }

  void resize(Image *img) {
    resize(img->get_data().get_number_of_rows(),
           img->get_data().get_number_of_columns());
  }

  //! Adjusts the information of the imager header taking into account the
  //! dimensions of the data and setting the time, date, type, etc ...
  void adjust_header() {
    header_.set_image_type(ImageHeader::IMG_IMPEM);
    header_.set_number_of_slices(1.0);
    header_.set_number_of_rows(data_.get_number_of_rows());
    header_.set_number_of_columns(data_.get_number_of_columns());
    header_.set_time();
    header_.set_date();
    header_.set_title(name_);
    header_.set_header(); // Initialize header
  }

  //! Reads the image from the file
  void read(String filename,const em::ImageReaderWriter<double> &reader) {
    IMP_LOG(VERBOSE, "reading EM image " << std::endl);
    reader.read(filename,header_,data_);
  }

  //! Reads and casts the image from the file (the image matrix of data must
  //! be stored as floats)
  void read_from_floats(String filename,
                            const em::ImageReaderWriter<double>& reader) {
    reader.read_from_floats(filename,header_,data_);
  }

  //! Reads and casts the image from the file (the image matrix
  //!  must be stored as ints)
  void read_from_ints(String filename,
                      const em::ImageReaderWriter<double>& reader) {
    reader.read_from_ints(filename,header_,data_);
  }


  //! Writes the image to a file
  void write(String filename,
             const em::ImageReaderWriter<double>& writer) {
    adjust_header(); // adjust the header to guarantee consistence
    writer.write(filename,header_,data_);
  }

  //! Writes the image to a file (the image matrix of data is stored as floats
  //! when writing)
  void write_to_floats(String filename,
                       const em::ImageReaderWriter<double>& writer) {
    adjust_header(); // adjust the header to guarantee consistence
    writer.write_to_floats(filename,header_,data_);
  }

  //! Writes the image to a file (the image matrix of data is stored as ints)
  void write_to_ints(String filename,
                     em::ImageReaderWriter<double>& writer) {
    adjust_header(); // adjust the header to guarantee consistence
    writer.write_to_floats(filename,header_,data_);
  }


  void show(std::ostream& out) const {
    out << "Image name   : " << name_ ;
    header_.show(out);
  }


  //! Define the basic things needed by any Object
  IMP_OBJECT_INLINE(Image, show(out), {});


protected:

  //! Name of the image. Frequently it will be the name of the file
  String name_;
  //! Matrix with the data for the image
  algebra::Matrix2D<double> data_;
  //! Header for the image with all the pertinent information
  ImageHeader header_;

}; // Image

//! A vector of reference counted pointers to EM images of type double
IMP_OBJECTS(Image,Images);

IMP_OUTPUT_OPERATOR(Image);

//! Reads images from files (For compatibility with SPIDER format,
//! the images are read from floats)
/**
  \param[in] names filenames of the images
  \param[in] rw  reader/writer to use
**/
IMPEMEXPORT Images read_images(Strings names,
                              const ImageReaderWriter<double> &rw);


//! Saves images to files (For compatibility with SPIDER format,
//! the images are written to floats)
/**
  \param[in] images Images to save
  \param[in] names filenames of the images
  \param[in] rw  reader/writer to use
**/
IMPEMEXPORT void save_images(Images images, Strings names,
                             const ImageReaderWriter<double> &rw);

IMPEM_END_NAMESPACE

#endif  /* IMPEM_IMAGE_H */
