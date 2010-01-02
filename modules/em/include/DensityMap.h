/**
 *  \file DensityMap.h
 *  \brief Class for handling density maps.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_DENSITY_MAP_H
#define IMPEM_DENSITY_MAP_H

#include "config.h"
#include "DensityHeader.h"
#include "MapReaderWriter.h"
#include <IMP/Object.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/VectorOfRefCounted.h>
#include <iostream>
#include <iomanip>

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
IMPEMEXPORT algebra::BoundingBox3D
   get_bounding_box(DensityMap* m, Float threshold);
//!
/**
\param[in] m a density map
\param[in] threshold consider volume of only voxels above this threshold
\note The method assumes 1.21 cubic A per dalton (Harpaz 1994).
 */
IMPEMEXPORT Float approximate_molecular_mass(DensityMap* m, Float threshold);


//! Class for handling density maps.
/** CONVENTIONS
    1) IT IS ASSUMED THAT THE LOCATION OF A VOXEL IS AT THE LOWER XYZ
       VALUES THAT IT DEFINES AS A BOX.
       This is important for function calc_all_voxel2loc()
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
  virtual ~DensityMap();
  DensityMap&  operator=(const DensityMap &other );
  //! Creates a new map with the given dimension
  /**
    param[in] nx x-dimension (voxels)
    param[in] ny y-dimension (voxels)
    param[in] nz z-dimension (voxels)
   */
  void CreateVoidMap(const int &nx,const int &ny,const int &nz);

#ifndef IMP_DEPRECATED
  /** \deprecated Use read() instead.*/
  void Read(const char *filename, MapReaderWriter &reader);
  /** \deprecated Use write() instead.*/
  void Write(const char *filename,MapReaderWriter &writer);
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

  //! Calculate the location of a given voxel.
  /** \param[in] index The voxel index
      \param[in] dim   The dimesion of intereset ( between x:=0,y:=1,z:=2)
      \return the location (x,y,z) (in angstroms) of a given voxel. If the
              index is not part of the map, the function returns -1.
      \todo change to const and throw exception if loc_calculated == false
   */
  float voxel2loc(const int &index,int dim) const;


  //! Calculate the voxel of a given xyz indexes
  /** \param[in] x The voxel index on the x axis of the grid
      \param[in] y The voxel index on the y axis of the grid
      \param[in] z The voxel index on the z axis of the grid
      \return the voxel index.
   */
  long xyz_ind2voxel(int x,int y,int z) const;

  //! Calculate the voxel of a given location
  /** \param[in] x The position ( in angstroms) of the x coordinate
      \param[in] y The position ( in angstroms) of the y coordinate
      \param[in] z The position ( in angstroms) of the z coordinate
      \exception  IndexException  The point is not covered by the grid.
      \return the voxel index of a given position. If the position is out of
              the boundaries of the map, the function returns -1.
   */
  long loc2voxel(float x, float y, float z) const;
  long loc2voxel(const algebra::Vector3D &v) const {
    return loc2voxel(v[0],v[1],v[2]);
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
  bool is_part_of_volume(const algebra::Vector3D &v) const {
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
  emreal get_value(const algebra::Vector3D &point) const {
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
  void set_origin(const IMP::algebra::Vector3D &v) {
    set_origin(v[0],v[1],v[2]);
  }

  algebra::Vector3D get_origin() const{
    return algebra::Vector3D(header_.get_origin(0),
                             header_.get_origin(1),
                             header_.get_origin(2));
  }

  algebra::Vector3D get_top() const {
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
              "x location requested before being calculated",
              InvalidStateException);
    return x_loc_;
  }
  //! Returns the y-location of the map
  /**
  \exception InvalidStateException The locations have not been calculated.
  */
  float* get_y_loc() const {
    IMP_USAGE_CHECK(loc_calculated_,
              "y location requested before being calculated",
              InvalidStateException);
    return y_loc_;
  }
  //! Returns the z-location of the map
  /**
  \exception InvalidStateException The locations have not been calculated.
  */
  float* get_z_loc() const {
    IMP_USAGE_CHECK(loc_calculated_,
              "z location requested before being calculated",
              InvalidStateException);
    return z_loc_;
  }

  emreal* get_data() const {return data_;}

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
  algebra::Vector3D get_centroid(emreal threshold=0.0) const;
  //! Returns the the value of the voxel with the highest density.
  emreal get_max_value() const;
  emreal get_min_value() const;
  //! Sums two grids.
  //! The result is kept in the map.
  //! The two maps should have the same dimensions and the same voxelsize
  /** \param[in] other the other map
   */
  void add(const DensityMap &other);

  long get_number_of_voxels() const;

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
  Float get_spacing() const {return header_.get_spacing();}
  //! Calculates the coordinates that correspond to all voxels.
  /** Can be precomputed to make corr faster.
      \todo which is a better design - have it public or call it from voxel2loc?
   */
  void calc_all_voxel2loc();

  virtual void show(std::ostream &out=std::cout) const {
    header_.show(out);
  }
  virtual VersionInfo get_version_info() const {
    return get_module_version_info();
  }

protected:

  void reset_voxel2loc();

  void allocated_data();
  void float2real(float *f_data, emreal **r_data);
  void real2float(emreal *r_data, float **f_data);

  DensityHeader header_; // holds all the info about the map
  emreal* data_; // the order is ZYX (Z-slowest)
  bool data_allocated_;

  //! Locations for each of the voxels of the map (they are precomputed and
  //! each one is of size nvox, being nvox the size of the map)
  float *x_loc_, *y_loc_, *z_loc_;
  //! true if the locations have already been computed
  bool loc_calculated_;

  bool normalized_;
  bool rms_calculated_;

};

inline algebra::BoundingBox3D get_bounding_box(const DensityMap *m) {
  return algebra::BoundingBox3D(m->get_origin(),
                       m->get_top());
}
//! rotate a grid
/**
/param[in] orig_dens the density map to rotate
/param[in] trans the transformation
\note this is a low resolution operation.
IMPEMEXPORT DensityMap* rotate_grid(const DensityMap *orig_dens,
                        const algebra::Transformation3D &trans);
*/
typedef VectorOfRefCounted<DensityMap*> DensityMaps;

IMPEM_END_NAMESPACE

#endif  /* IMPEM_DENSITY_MAP_H */
