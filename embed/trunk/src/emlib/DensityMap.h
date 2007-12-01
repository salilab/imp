#ifndef _DENSITYMAP_H
#define _DENSITYMAP_H


#include "EM_config.h"
#include "DensityHeader.h"
#include "Vector3.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "MapReaderWriter.h"

typedef float real;

//! Class for handeling density maps.
/*	- CONVENTIONS
	1) IT IS ASSUMED THAT THE LOCATION OF A VOXEL IS AT THE LOWER XYZ VALUES THAT IT DEFINES AS A BOX.
	This is important for function calc_all_voxel2loc() 
*/
class EMDLLEXPORT DensityMap
{
public:
  /** 
      
   */
  DensityMap();
  DensityMap(const DensityMap &other);
  DensityMap&  operator=(const DensityMap &other );
  ~DensityMap();

  /**
   */
  void CreateVoidMap(const int &nx,const int &ny,const int &nz);

  void Read(const char *filename, MapReaderWriter &reader);
  //  void Read(const string &filename, MapReaderWriter &reader);
  void Write(const char *filename,MapReaderWriter &writer);
  //  void Write(const string &filename, MapReaderWriter &writer);


  // data managment
  void reset_data(); 

  //! Calculates RMSD and mean of a map values are stored in the header. The header stores whether map is normalized.
  float calcRMS();


  //!normailze the density according to standard deviation (stdv). The mean is subtracted from the map, which is then divided by the stdv 
  // Flag normalize is set to avoid repeated computation
  void std_normalize();
  

  inline bool is_normalized() const {return normalized;}

//! Calculate the location of a given voxel.
/** \param[in] index The voxel index
    \param[in] dim   The dimesion of intereset ( between x:=0,y:=1,z:=2)
    \return the location (x,y,z) (in angstroms) of a given voxel. If the index is not part of the map, the function returns -1. 
*/
  float voxel2loc(const int &index,int dim);

//! Calculate the voxel of a given location
/** \param[in] x The position ( in angstroms) of the x coordinate
    \param[in] y The position ( in angstroms) of the y coordinate
    \param[in] z The position ( in angstroms) of the z coordinate
    \exception std::out_of_range The point is not covered by the grid.
    \return the voxel index of a given position. If the position is out of the boundaries of the map, the function returns -1.
*/
  int loc2voxel(float x, float y, float z) const;

//! Checks whether a given point is in the grid the voxel of a given location
/** \param[in] x The position ( in angstroms) of the x coordinate
    \param[in] y The position ( in angstroms) of the y coordinate
    \param[in] z The position ( in angstroms) of the z coordinate
    \return true if the point is part of the grid, false otherwise.
*/
  bool part_of_volume(float x,float y,float z) const;




  //! Sets the origin of the header
  void set_origin(float x,float y,float z);


  // inspection functions
  const DensityHeader *get_header()const {return &header;}
  DensityHeader *get_header_writable() {return &header;}
  
  float* get_x_loc() const {
    if (!loc_calculated) {
      std::cerr << "DensityMap::get_x_loc ask for x location although it has not been calculated " << std::endl;
      throw 1;
    }
    return x_loc;
  }
  float* get_y_loc() const {
    if (!loc_calculated) {
      std::cerr << "DensityMap::get_x_loc ask for x location although it has not been calculated " << std::endl;
      throw 1;
    }
    return y_loc;}
  float* get_z_loc() const {
    if (!loc_calculated) {
      std::cerr << "DensityMap::get_x_loc ask for x location although it has not been calculated " << std::endl;
      throw 1;
    }
    return z_loc;}
  float* get_data() const {return data;}


//! Checks if two maps have the same 
/** \param[in] other the map to compare with
    \return true if the two maps have the same origin
*/
  bool same_origin  (const DensityMap &other) const;
//! Checks if  two maps have the same dimensions
/** \param[in] other the map to compare with
    \return true if the two maps have the same dimensions
*/
  bool same_dimensions  (const DensityMap &other) const;
//! Checks if  two maps have the same voxel size
/** \param[in] other the map to compare with
    \return true if the two maps have the same voxel size
*/
  bool same_voxel_size  (const DensityMap &other) const;


protected:

  /**
     calculate the coordinates that correspond to all voxels.
    can be precomputed to make corr faster.  
  */
  void calc_all_voxel2loc(); // TODO - which is a better desing - have it public or call it from voxel2loc?


  void allocated_data();

  DensityHeader header;
  real *data; // the order is ZYX (Z-slowest)
  bool data_allocated;

  // Locations for each of the voxels of the map (they are precomputed and each one is of size nvox, being nvox the size of the map)
  real *x_loc,*y_loc,*z_loc;
  bool loc_calculated; // true if the locations have already been computed

  bool normalized;
  bool rms_calculated;

};



#endif //_DENSITYMAP_H
