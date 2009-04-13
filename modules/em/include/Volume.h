/**
 *  \file Volume.h
 *  \brief Management of IMP volumes for Electron Microscopy. Compatible with
 *  Spider and Xmipp formats
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_VOLUME_H
#define IMPEM_VOLUME_H

#include "config.h"
#include "ImageHeader.h"
// #include "EMresample.h"
#include "IMP/algebra/Matrix3D.h"
#include "IMP/algebra/Vector3D.h"
#include <typeinfo>
#include <complex>

IMPEM_BEGIN_NAMESPACE

//! Template class for managing 3D Electron Microscopy volumes in IMP
template <typename T>
class Volume
{
public:
  //! Empty constructor
  Volume() {
    data_.set_start(0,0);data_.set_start(1,0);data_.set_start(2,0);
    name_ = "";
    locations_calculated_ = false;
    normalized_ = false;
    rms_calculated_ = false;
  }

  //! Constructor with size
  Volume(int Zdim, int Ydim, int Xdim) {
    data_.resize(Zdim, Ydim, Xdim);
    data_.set_start(0,0);data_.set_start(1,0);data_.set_start(2,0);
    header_.set_header();
    locations_calculated_ = false;
    normalized_ = false;
    rms_calculated_ = false;
  }

  //! Return the matrix of data of the volume
  IMP::algebra::Matrix3D<T>& get_data() {
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
               typeid(T) == typeid(float)) {
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

  //! Reads the volume from the file
  /**
   * \param[in] filename name of the file
   * \param[in] skip_type_check true if the check for type of image is skipped
   * \param[in] force_reversed true to force reverse reading
   * \param[in] skip_extra_checkings true to skip the extra stringent tests
   */
  void read(String filename,bool skip_type_check,
        bool force_reversed,bool skip_extra_checkings) {
    std::ifstream in;
    in.open(filename.c_str(), std::ios::in | std::ios::binary);
    header_.read(in,skip_type_check,force_reversed,skip_extra_checkings);
    data_.resize(header_.get_slices(),
                 header_.get_rows(),
                 header_.get_columns());
    data_.read_binary(in);
    in.close();
  }

  //! Writes the volume to a file
  void write(String filename, bool force_reversed = false) {
    adjust_header(); // First adjust the header to guarantee consistence
    std::ofstream out;
    out.open(filename.c_str(), std::ios::out | std::ios::binary);
    header_.write(out, force_reversed);
    data_.write_binary(out);
    out.close();
  }

  //! Clears all the data in the volume and sets it to the given value
  /**
   * \param[in] val value to set
   */
  void reset_data(float const val) {
    for(int i=data_.get_start(0);i<= data_.get_finish(0);i++) {
      for(int j=data_.get_start(1);j<= data_.get_finish(1);j++) {
        for(int k=data_.get_start(2);k<= data_.get_finish(2);k++) {
          data_(i,j,k)=val;
        }
      }
    }
  }

  //! Computes the x,y,z coordinates (location) associated with each voxel of
  //! the volume and stores it in the locations_ matrix.
  void compute_locations() {
    if (locations_calculated_)
      return;

    int nz = (int)data_.get_slices();
    int ny = (int)data_.get_rows();
    int nx = (int)data_.get_columns();
    float pixel_size = header_.get_object_pixel_size();

    locations_.resize(nz, ny, nx);

    for (int k = 0;k <= nz;k++) {
      for (int j = 0;j <= ny;j++) {
        for (int i = 0;i <= nx;i++) {
          IMP::algebra::Vector3D p;
          p[2] = i * pixel_size + header_.get_xorigin();
          p[1] = j * pixel_size + header_.get_yorigin();
          p[0] = k * pixel_size + header_.get_zorigin();
          locations_(k, j, i) = p;
        }
      }
    }
    locations_calculated_ = true;
  }

  //! Returns the number of voxels of the map
  long get_number_of_voxels() const {
    return data_.get_slices() * data_.get_rows() * data_.get_columns();
  }

  //! Returns true if the physical index for the element of the matrix
  //! that contains the data in the map is within the range.
  /**
   * \param[in] i index for dimension 0 (z)
   * \param[in] j index for dimension 1 (y)
   * \param[in] k index for dimension 2 (x)
   */
  bool is_xyz_ind_part_of_volume(int i,int j,int k) const {
    std::vector<int> v(3);
    v[0]=i;v[1]=j;v[2]=k;
    return data_.is_physical_element(v);
  }

  //! Returns true if coordinates of a given point belong to those covered
  //! by the volume
  /**
   * \param[in] x coordinate for x
   * \param[in] y coordinate for y
   * \param[in] z coordinate for z
   */
  bool is_part_of_volume(T x,T y,T z) const {
    float pixel_size = header_.get_object_pixel_size();
    IMP::algebra::Vector3D origin(header_.get_xorigin(),
                                  header_.get_yorigin(),
                                  header_.get_zorigin());
    IMP::algebra::Vector3D v((T)data_.get_columns(),
                             (T)data_.get_rows(),
                             (T)data_.get_slices());
    IMP::algebra::Vector3D end= origin + pixel_size * v;
    for (int i = 0;i<3;i++) {
      if(v[i]<origin[i] || end[i]<v[i]) {
        return false;
      }
    }
    return true;
  }

//  friend void resample(Volume volume, const ParticlesAccessPoint& access_p);

protected:
  //! Name of the volume. Frequently it will be the name of the file
  String name_;
  //! Matrix3D with the data for each voxel of the volume
  IMP::algebra::Matrix3D<T> data_;
  //! Header for the volume with all the pertinent information
  ImageHeader header_;
  //! Matrix with the locations (z,y,x) of the volume voxels
  IMP::algebra::Matrix3D<IMP::algebra::Vector3D> locations_;
  //! True if the locations have being calculated
  bool locations_calculated_;
  //! True if the volume values have been normalized
  bool normalized_;
  //! True if the rms of the values in the volume have been computed
  bool rms_calculated_;

}; // Volume

IMPEM_END_NAMESPACE
#endif  /* IMPEM_VOLUME_H */
