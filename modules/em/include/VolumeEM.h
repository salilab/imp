/**
 *  \file VolumeEM.h
 *  \brief Management of IMP volumes for Electron Microscopy
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_VOLUME_EM_H
#define IMPEM_VOLUME_EM_H

#include <typeinfo>
#include "config.h"
#include "ImageHeader.h"
// #include "EMresample.h"
#include "IMP/algebra/Matrix3D.h"
#include "IMP/algebra/Vector3D.h"

IMPEM_BEGIN_NAMESPACE

//! Template class for managing 3D Electron Microscopy volumes in IMP
template<typename T>
class IMPEMEXPORT VolumeEM
{
public:
  //! Empty constructor
  VolumeEM() {
    name_ = "";
    locations_calculated_ = false;
    normalized_ = false;
    rms_calculated_ = false;
  }

  //! Constructor with size
  VolumeEM(Int Zdim, Int Ydim, Int Xdim) {
    data_.resize(Zdim, Ydim, Xdim);
    header_.set_header();
    locations_calculated_ = false;
    normalized_ = false;
    rms_calculated_ = false;
  }

  //! Return the matrix of data of the volume
  algebra::Matrix3D<T>& get_data() {
    return data_;
  }

  //! Return the header of the volume
  ImageHeader& get_header() {
    return header_;
  }

  //! Adjusts the information of the imager header taking into account the
  //! dimensions of the data and setting the time, date, type, etc ...
  void adjust_header() {
    if (typeid(T) == typeid(std::complex< double >)) {
      header_.set_image_type(ImageHeader::VOL_FOURIER);
    } else if (typeid(T) == typeid(double) ||
               typeid(T) == typeid(Float)) {
      header_.set_image_type(ImageHeader::VOL_IMPEM);
    }
    header_.set_slices(data_.get_slices());
    header_.set_rows(data_.get_rows());
    header_.set_columns(data_.get_columns());
    header_.set_time();
    header_.set_date();
    header_.set_title(name_);
    // Make the header consistent given all the new set data
    header_.set_header();
  }

  //! Reads the image from the file
  void read(String filename) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    header_.read(in);
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

  //! Clears all the data in the volume and sets it to zeros
  void reset_data() {
    data_.init_zeros();
  }

  //! Computes the x,y,z coordinates (location) associated with each voxel of
  //! the volume and stores it in the locations_ matrix.
  void compute_locations() {
    if (locations_calculated_)
      return;

    Int nz = (Int)data_.get_slices();
    Int ny = (Int)data_.get_rows();
    Int nx = (Int)data_.get_columns();
    Float pixel_size = header_.get_object_pixel_size();

    locations_.resize(nz, ny, nx);

    for (Int k = 0;k <= nz;k++) {
      for (Int j = 0;j <= ny;j++) {
        for (Int i = 0;i <= nx;i++) {
          algebra::Vector3D p;
          p[2] = i * pixel_size + header_.get_xorigin();
          p[1] = j * pixel_size + header_.get_yorigin();
          p[0] = k * pixel_size + header_.get_zorigin();
          locations_(k, j, i) = p;
        }
      }
    }
    locations_calculated_ = true;
  }


//  friend void resample(VolumeEM volume, const ParticlesAccessPoint& access_p);


protected:
  //! Name of the volume. Frequently it will be the name of the file
  String name_;
  //! Matrix3D with the data for each voxel of the volume
  algebra::Matrix3D<T> data_;
  //! Header for the volume with all the pertinent information
  ImageHeader header_;
  //! Matrix with the locations (z,y,x) of the volume voxels
  algebra::Matrix3D<algebra::Vector3D> locations_;
  //! True if the locations have being calculated
  bool locations_calculated_;
  //! True if the volume values have been normalized
  bool normalized_;
  //! True if the rms of the values in the volume have been computed
  bool rms_calculated_;

}; // VolumeEM

IMPEM_END_NAMESPACE
#endif  /* IMPEM_VOLUME_EM_H */
