/**
 *  \file DensityMap.h
 *  \brief Class for handling density maps.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_DENSITY_MAP_H
#define IMPEM_DENSITY_MAP_H

#include "em_config.h"
#include "DensityHeader.h"
#include "MapReaderWriter.h"
#include <IMP/Object.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/VectorOfRefCounted.h>
#include <boost/scoped_array.hpp>
#include <iostream>
#include <iomanip>
//#include <IMP/statistics/Histogram.h>

IMPEM_BEGIN_NAMESPACE

class DensityMap;

/** Read a density map from a file and return it.
    \relatesalso DensityMap
*/
IMPEMEXPORT DensityMap* read_map(const char *filename, MapReaderWriter &reader);

/** Read a density map from a file and return it. Guess the file type from the
    file name. The file formats supported are:
    - .mrc for MRC files
    \relatesalso DensityMap
*/
IMPEMEXPORT DensityMap* read_map(const char *filename);


/** Write a density map to a file.
    \relatesalso DensityMap
*/
IMPEMEXPORT void write_map(DensityMap* m, const char *filename,
                           MapReaderWriter &writer);
//!
/**
\param[in] m a density map
\param[in] threshold find the boudning box for voxels
           with value above the threshold
 */
IMPEMEXPORT algebra::BoundingBoxD<3>
   get_bounding_box(const DensityMap* m, Float threshold);
//!
/**
\param[in] m a density map
\param[in] threshold consider volume of only voxels above this threshold
\note The method assumes 1.21 cubic A per dalton (Harpaz 1994).
 */
IMPEMEXPORT Float approximate_molecular_mass(DensityMap* m, Float threshold);


//! Class for handling density maps.
/** /note The location of a voxel is its center. That is important
     for sampling function as well as for functions
     like get_location_in_dim_by_voxel.
 */
class IMPEMEXPORT DensityMap: public Object
{
  IMP_NO_SWIG(friend IMPEMEXPORT DensityMap* read_map(const char *filename,
                                          MapReaderWriter &reader));
  IMP_NO_SWIG(friend IMPEMEXPORT void write_map(DensityMap* m,
                                                const char *filename,
                                                MapReaderWriter &writer));

public:
  DensityMap();
  DensityMap(const DensityMap &other);
  //! Construct a density map as intructed in the input header
  DensityMap(const DensityHeader &header);
  DensityMap&  operator=(const DensityMap &other );

#ifndef IMP_DOXYGEN
#ifndef IMP_DEPRECATED
  /** \deprecated Use read() instead.*/
  void Read(const char *filename, MapReaderWriter &reader);
  /** \deprecated Use write() instead.*/
  void Write(const char *filename,MapReaderWriter &writer);
#endif
#endif


  //!  Set the density voxels to some calue and reset the managment flags.
  /**
  \param[in] value all of the density voxels will have this value
   */
  void reset_data(float value=0.0);

  //! Calculates RMSD and mean of a map values are stored in the header.
  /** The header stores whether map is normalized.
   */
  emreal calcRMS();


  //! Normailze the density voxles according to standard deviation (stdv).
  /** The mean is subtracted from the map, which is then divided by the stdv.
      The normalization flag is set to avoid repeated computation */
  void std_normalize();

  inline bool is_normalized() const {return normalized_;}

  //! Calculate the location of a given voxel in a given dimension
  /** \param[in] index The voxel index
      \param[in] dim   The dimesion of intereset (x:=0,y:=1,z:=2)
      \return the location (x,y,z) (in angstroms) of a given voxel. If the
              index is not part of the map, the function returns -1.
   */
  float get_location_in_dim_by_voxel(long index,int dim) const;


  //! Calculate the voxel of a given xyz indexes
  /** \param[in] x The voxel index on the x axis of the grid
      \param[in] y The voxel index on the y axis of the grid
      \param[in] z The voxel index on the z axis of the grid
      \return the voxel index.
   */
  inline long xyz_ind2voxel(int x,int y,int z) const {
  return z * header_.get_nx() * header_.get_ny() +
         y * header_.get_nx() + x;

  }

  //! Calculate the voxel of a given location
  /** \param[in] x The position (in angstroms) of the x coordinate
      \param[in] y The position (in angstroms) of the y coordinate
      \param[in] z The position (in angstroms) of the z coordinate
      \return the voxel index of a given position. If the position is out of
              the boundaries of the map, the function returns -1.
   */
  long get_voxel_by_location(float x, float y, float z) const;

  //! Calculate the voxel of a given location
  /** \param[in] v The position (in angstroms)
      \return the voxel index of a given position. If the position is out of
              the boundaries of the map, the function returns -1.
   */
  long get_voxel_by_location(const algebra::VectorD<3> &v) const{
    return get_voxel_by_location(v[0],v[1],v[2]);
  }
  //! Calculate dimension index of a given location
  /** \param[in] v The position (in angstroms)
      \param[in] ind dimension index (X:0,Y:1 or Z:2)
   */
  int get_dim_index_by_location(const algebra::VectorD<3> &v,int ind) const;

  algebra::VectorD<3> get_location_by_voxel(long index) const {
    return algebra::Vector3D(get_location_in_dim_by_voxel(index,0),
                             get_location_in_dim_by_voxel(index,1),
                             get_location_in_dim_by_voxel(index,2));
  }

  bool is_xyz_ind_part_of_volume(int ix,int iy,int iz) const;

  //! Checks whether a given point is in the grid the voxel of a given location
  /** \param[in] x The position ( in angstroms) of the x coordinate
      \param[in] y The position ( in angstroms) of the y coordinate
      \param[in] z The position ( in angstroms) of the z coordinate
      \return true if the point is part of the grid, false otherwise.
   */
  bool is_part_of_volume(float x,float y,float z) const;
  //! Checks whether a given point is in the grid the voxel of a given location
  /** \param[in] v The position ( in angstroms)
      \return true if the point is part of the grid, false otherwise.
   */
  bool is_part_of_volume(const algebra::VectorD<3> &v) const {
    return is_part_of_volume(v[0],v[1],v[2]);
  }

  //! Gets the value of the voxel located at (x,y,z)
  /** \param[in] x The position ( in angstroms) of the x coordinate
      \param[in] y The position ( in angstroms) of the y coordinate
      \param[in] z The position ( in angstroms) of the z coordinate
      \return the value of the voxel located at (x,y,z)
      \exception IndexException The point is not covered by the grid.
   */
  emreal get_value(float x,float y,float z) const;
  emreal get_value(const algebra::VectorD<3> &point) const {
    return get_value(point[0],point[1],point[2]);
  }


  //! Gets the value of the voxel at a given index
  /**
    \param[in] index voxel number in physical sense, NOT logical
  */
  emreal get_value(long index) const;


  //! Set the value of the voxel at a given index
  /**
    \param[in] index voxel number in physical sense, NOT logical
    \param[in] value value
  */
  void set_value(long index,emreal value);


  //! Set the value of the voxel at a given index
  /**
    index voxel number in physical sense, NOT logical
  */
  void set_value(float x, float y, float z,emreal value);

  //! Sets the origin of the header
  /**
    \param x the new x (angstroms)
    \param y the new y (angstroms)
    \param z the new z (angstroms)
  */
  void set_origin(float x,float y,float z);
  void set_origin(const IMP::algebra::VectorD<3> &v) {
    set_origin(v[0],v[1],v[2]);
  }

  algebra::VectorD<3> get_origin() const{
    return algebra::Vector3D(header_.get_origin(0),
                             header_.get_origin(1),
                             header_.get_origin(2));
  }

  algebra::VectorD<3> get_top() const {
    return algebra::Vector3D(header_.get_top(0),
                             header_.get_top(1),
                             header_.get_top(2));
  }

  // inspection functions
  const DensityHeader *get_header()const {return &header_;}
  //! Returns a pointer to the header of the map in a writable version
  DensityHeader *get_header_writable() {return &header_;}

  //! Returns the x-location of the map
  /**
  \exception InvalidStateException The locations have not been calculated.
  */
  float* get_x_loc() const {
    IMP_USAGE_CHECK(loc_calculated_,
              "x location requested before being calculated");
    return x_loc_.get();
  }
  //! Returns the y-location of the map
  /**
  \exception InvalidStateException The locations have not been calculated.
  */
  float* get_y_loc() const {
    IMP_USAGE_CHECK(loc_calculated_,
              "y location requested before being calculated");
    return y_loc_.get();
  }
  //! Returns the z-location of the map
  /**
  \exception InvalidStateException The locations have not been calculated.
  */
  float* get_z_loc() const {
    IMP_USAGE_CHECK(loc_calculated_,
              "z location requested before being calculated");
    return z_loc_.get();
  }

  emreal* get_data() const {return data_.get();}

  //! Checks if two maps have the same origin
  /** \param[in] other the map to compare with
      \return true if the two maps have the same origin
   */
  bool same_origin(const DensityMap &other) const;

  //! Checks if  two maps have the same dimensions
  /** \param[in] other the map to compare with
      \return true if the two maps have the same dimensions
   */
  bool same_dimensions(const DensityMap &other) const;

  //! Checks if  two maps have the same voxel size
  /** \param[in] other the map to compare with
      \return true if the two maps have the same voxel size
   */
  bool same_voxel_size(const DensityMap &other) const;
  //! Calculates the centroid of all the voxels with
  //! density above a given threshold
  /** \param[in] threshold the input threshold
  */
  algebra::VectorD<3> get_centroid(emreal threshold=0.0) const;
  //! Returns the the value of the voxel with the highest density.
  emreal get_max_value() const;
  //! Returns the the value of the voxel with the lowest density.
  emreal get_min_value() const;
  //! Sums two grids.
  //! The result is kept in the map.
  /** \param[in] other the other map
      \note The shared extend is sumed
      \note The two maps should have the same voxelsize and other
            should be contained within this map
   */
  void add(const DensityMap &other);

  //! Pick the max value between two corresponding voxels between two maps
  //! The result is kept in the map.
  /** \param[in] other the other map
      \note The two maps should have the same voxelsize and the same dimensions
   */
  void pick_max(const DensityMap &other);

  long get_number_of_voxels() const;

  //! Set the map dimension and reset all voxels to 0
  /**
    \param[in] nx x-dimension (voxels)
    \param[in] ny y-dimension (voxels)
    \param[in] nz z-dimension (voxels)
    \note the origin and spacing remain unchanged
   */
  void set_void_map(int nx,int ny,int nz);


  //! Increase the dimension of the map
  //! The function pads zeros to the  right-upper section on the map.
  //! The original content of the map will be in the lower XYZ part of the map
  /** \param[in] nx the number of voxels on the X axis
      \param[in] ny the number of voxels on the Y axis
      \param[in] nz the number of voxels on the Z axis
      \param[in] val   all additional voxels will have this value
      \exception if the input  x/y/z voxels is smaller than the one
                 currently in the map
   */
  void pad(int nx, int ny, int nz,float val=0.0);

  //! Create a new padded map
  /** \brief Given this map of size [nx,ny,nz],
   the new map is of size [2*mrg_x+nx,2*mrg_y+ny,2*mrg_z+nz].
   The new map will consist of the values of the old map,
   padded margin on all sides.
   \param[in] mrg_x
      number of margin voxels to add on both right and left on the X axis
   \param[in] mrg_y
      number of margin voxels to add on both right and left on the Y axis
   \param[in] mrg_z
      number of margin voxels to add on both right and left on the Z axis
   \param[in] val   all additional voxels will have this value
   \exception if the input  x/y/z voxels is smaller than the one
              currently in the map
   */
  DensityMap* pad_margin(int mrg_x, int mrg_y, int mrg_z,float val=0.0);


  //! Create a new cropped map
  /** \brief The margins are determined to be the bounding box
             with density values below the input
   \param[in] threshold used for cropping
   */
  DensityMap* get_cropped(float threshold);
  //! Create a new cropped map with the bounding box extent
  /**
     \param[in] bb the bounding box
     \note If the input bounding box is larger than the density box,
           it is snapped to the right size.
   */
  DensityMap* get_cropped(const algebra::BoundingBox3D &bb);

  //! Get the maximum value in a XY plane indicated by a Z index
  float get_maximum_value_in_xy_plane(int z_ind);
  //! Get the maximum value in a XZ plane indicated by a Y index
 float get_maximum_value_in_xz_plane(int y_ind);
  //! Get the maximum value in a YZ plane indicated by a X index
 float get_maximum_value_in_yz_plane(int x_ind);

  //! Multiply each voxel in the map by the input factor
  //! The result is kept in the map.
  /** \param[in] factor the multiplication factor
   */
  void multiply(float factor);

  //! Prints the locations of all of the voxels with value above a given
  //! threshold into the input stream.
  std::string get_locations_string(float t);

  //! Updated the voxel size of the map
  void update_voxel_size(float new_apix);

  //! Updated the voxel size of the map
  /** \note Use update_voxel_size() to set the spacing value.
  */
  Float get_spacing() const {return header_.get_spacing();}
  //! Calculates the coordinates that correspond to all voxels.
  void calc_all_voxel2loc();

  IMP_OBJECT_INLINE(DensityMap, header_.show(out),{});
  //! copy map into this map
  void copy_map(const DensityMap &other);
  //! Convolution a kernel with the map
  /**
\param[in] kernel an array of kernel values. The data is in ZYX
                   order, Z is the slowest.
\param[in] lenght the array leght
   */
  void convolute_kernel(double *kernel, int dim_len){
    //todo - add a test that lenght is even
    IMP_USAGE_CHECK((dim_len*dim_len*dim_len)>1,"The input lenght is wrong\n");
    unsigned int margin=(dim_len-1)/2;
    //smooth the density using the kernel
    float val;
    int kernel_ind,map_ind;
  for (unsigned int iz=margin;iz<header_.get_nz()-margin;iz++) {
    map_ind=iz*header_.get_ny()*header_.get_nx();
    for (unsigned int iy=margin;iy<header_.get_ny()-margin;iy++) {
      map_ind+=iy*header_.get_nx();
      for (unsigned int ix=margin;ix<header_.get_nx()-margin;ix++) {
        map_ind += ix;
        val = data_[map_ind];
        if (val>EPS) { //smooth this value
         for (int iz2=-margin;iz2<=static_cast<int>(margin);iz2++) {
            kernel_ind=iz2*dim_len*dim_len;
            for (int iy2=-margin;iy2<=static_cast<int>(margin);iy2++){
             kernel_ind+=iy2*dim_len;
             for (int ix2=-margin;ix2<=static_cast<int>(margin);ix2++) {
                kernel_ind+=ix2;
                data_[map_ind]+=val*kernel[kernel_ind];
             }}} // for iz2,iy2,ix2
          }//if val>EPS
      }}} // for iz,iy,ix
}
  int lower_voxel_shift(emreal loc, emreal kdist, emreal orig, int ndim) const;
  int upper_voxel_shift(emreal loc, emreal kdist, emreal orig, int ndim) const;
protected:
  int get_dim_index_by_location(float loc_val,
                              int ind) const;
  //!update the header values  -- still in work
  void update_header();
  void reset_all_voxel2loc();

  void allocated_data();
  void float2real(float *f_data, boost::scoped_array<emreal> &r_data);
  void real2float(emreal *r_data, boost::scoped_array<float> &f_data);

  DensityHeader header_; // holds all the info about the map
  boost::scoped_array<emreal> data_; // the order is ZYX (Z-slowest)
  bool data_allocated_;

  //! Locations for each of the voxels of the map (they are precomputed and
  //! each one is of size nvox, being nvox the size of the map)
  boost::scoped_array<float> x_loc_, y_loc_, z_loc_;
  //! true if the locations have already been computed
  bool loc_calculated_;

  bool normalized_;
  bool rms_calculated_;

};

inline algebra::BoundingBoxD<3> get_bounding_box(const DensityMap *m) {
  const DensityHeader *h=m->get_header();
  return algebra::BoundingBoxD<3>(
     m->get_origin(),
     m->get_origin()+algebra::Vector3D(m->get_spacing()*h->get_nx(),
                                       m->get_spacing()*h->get_ny(),
                                       m->get_spacing()*h->get_nz()));
}
//! Create an empty density map from a boudning box
IMPEMEXPORT DensityMap *create_density_map(
                                           const algebra::BoundingBox3D &bb,
                                           double spacing);
//! Create an empty density map
IMPEMEXPORT DensityMap *create_density_map(
                                           int nx,int ny,int nz,
                                           double spacing);
 //! Calculate a bounding box around a 3D point within the EM grid
 /**
\param[in] d_map the density map
\param[in] point (x,y,z) coordinates of the point to sample around
\param[in] kdist the lenght of the box
\param[out] iminx the minimum index on the X axis of the output bounding box
\param[out] iminy the minimum index on the Y axis of the output bounding box
\param[out] iminz the minimum index on the Z axis of the output bounding box
\param[out] imaxx the maximum index on the X axis of the output bounding box
\param[out] imaxy the maximum index on the Y axis of the output bounding box
\param[out] imaxz the maximum index on the Z axis of the output bounding box
  */
inline void calc_local_bounding_box(
                   const em::DensityMap &d_map,
                   double x,double y, double z,
                   float kdist,
                   int &iminx,int &iminy, int &iminz,
                   int &imaxx,int &imaxy, int &imaxz) {
  const DensityHeader *h=d_map.get_header();
  iminx = d_map.lower_voxel_shift(x, kdist,h->get_xorigin(),h->get_nx());
  iminy = d_map.lower_voxel_shift(y, kdist,h->get_yorigin(),h->get_ny());
  iminz = d_map.lower_voxel_shift(z, kdist,h->get_zorigin(),h->get_nz());
  imaxx = d_map.upper_voxel_shift(x, kdist,h->get_xorigin(),h->get_nx());
  imaxy = d_map.upper_voxel_shift(y, kdist,h->get_yorigin(),h->get_ny());
  imaxz = d_map.upper_voxel_shift(z, kdist,h->get_zorigin(),h->get_nz());
}

//! rotate a grid
/**
/param[in] orig_dens the density map to rotate
/param[in] trans the transformation
\note this is a low resolution operation.
IMPEMEXPORT DensityMap* rotate_grid(const DensityMap *orig_dens,
                        const algebra::Transformation3D &trans);
*/

IMP_OBJECTS(DensityMap,DensityMaps);

/** Return the value for the density map, m, at point v, interpolating linearly
    from the sample values. The resulting function is C0 over R3.
    \relatesalso DensityMap
*/
IMPEMEXPORT double get_density(const DensityMap *m,
                               const algebra::VectorD<3> &v);

/** Return a new density map containing a rotated version of the old
    one. Only voxels whose value is above threshold are considered when
    computing the bounding box of the new map (set IMP::em::get_bounding_box()).
    \relatesalso DensityMap
*/
IMPEMEXPORT DensityMap* get_transformed(const DensityMap *in,
                                        const algebra::Transformation3D &tr,
                                        double threshold);

/** Return a new density map containing a rotated version of the old
    one. The dimension of the new map is the same as the old one.
    \relatesalso DensityMap
*/
IMPEMEXPORT DensityMap* get_transformed(DensityMap *in,
                                        const algebra::Transformation3D &tr);


/** Get a resampled version of the map. The spacing is multiplied by scaling.
    That means, scaling values greater than 1 increase the voxel size.
    \relatesalso DensityMap
*/
IMPEMEXPORT DensityMap* get_resampled(DensityMap *in, double scaling);


//! Rotate a density map into another maps
/**
\param[in] from the map to transform
\param[in] tr transform the from density map by this transformation
\param[out] into the map to tranform into
\param[in] calc_rms if true RMS is calculated on the transformed map
 \relatesalso DensityMap
*/
IMPEMEXPORT void get_transformed_into(const DensityMap *from,
                                      const algebra::Transformation3D &tr,
                                      DensityMap *into,
                                       bool calc_rms=true);

inline bool get_interiors_intersect(const DensityMap *d1,
                                    const DensityMap *d2){
  return get_interiors_intersect(get_bounding_box(d1),
                                 get_bounding_box(d2));
}
//! Get a histrogram of density values
/**
\param[in] dmap the density map to analyse
\param[in] threshold only add voxels with value above this threshold
                     to the histogram
\param[in] num_bins the number of bins to have in the histogram
 \relatesalso DensityMap
*/
// IMPEMEXPORT statistics::Histogram
// get_density_histogram(const DensityMap *dmap, float threshold,int num_bins);


//! Get a segment of the map according to xyz indexes
/**
\note the output map will be cover
the region [[nx_start,nx_end],[]ny_start,ny_end,[nz_start,nz_end]]
 */
IMPEMEXPORT DensityMap* get_segment(DensityMap *map_to_segment,
                                    int nx_start,int nx_end,
                                    int ny_start,int ny_end,
                                    int nz_start,int nz_end);

//! Return a map with 0 for all voxels below the
//! threshold and 1 for thoes above
IMPEMEXPORT DensityMap* binarize(DensityMap *orig_map,
                                 float threshold);
//! Return a density map for which voxel i contains the result of
//! m1[i]*m2[i]. The function assumes m1 and m2 are of the same dimensions.
IMPEMEXPORT DensityMap* multiply(const DensityMap *m1,
                                 const DensityMap *m2);
//! Return a convolution between density maps m1 and m2.
//! The function assumes m1 and m2 are of the same dimensions.
IMPEMEXPORT double convolute(const DensityMap *m1,const DensityMap *m2);
IMPEM_END_NAMESPACE

#endif  /* IMPEM_DENSITY_MAP_H */
