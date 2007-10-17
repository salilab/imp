#ifndef _DENSITYMAP_H
#define _DENSITYMAP_H


/*
 *  DensityMap.h
 *  EM
 *  
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */


#include "DensityHeader.h"
#include "Vector3.h"
#include <iostream>
#include <iomanip>
#include "MapReaderWriter.h"
using std::istream;
using std::ios;

typedef float real;


class DensityMap
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
  void Write(const char *filename,MapReaderWriter &writer);


  // data managment
  void ResetData() {
    for (int i=0;i<header.nx*header.ny*header.nz;i++) {
      data[i]=0.0;
    }
  }

  /*calculate RMSD and mean of a map
    values are stored in the header.
    The header stores whether map is normalized. */
  float calcRMS();


  /*normailze the density according to standard deviation (stdv).
    The mean is subtracted from the map, which is then divided by the stdv */
  void stdNormalize();
  
  // voxel inspection
  float voxel2loc(const int &index,int dim);

  // inspection functions
  const DensityHeader &get_header()const {return header;}
  DensityHeader &get_header_writable() {return header;}
  
  float* get_x_loc() const {return x_loc;}
  float* get_y_loc() const {return y_loc;}
  float* get_z_loc() const {return z_loc;}
  float* get_data() const {return data;}



protected:

  /**
     calculate the coordinates that correspond to all voxels.
    can be precomputed to make corr faster.  
  */
  void calc_all_voxel2loc(); // TODO - which is a better desing - have it public or call it from voxel2loc?


  /**
     allocated data
  */
  void allocated_data();
  DensityHeader header;
  real *data; // the order is ZYX (Z-slowest)
  bool data_allocated;

  real *x_loc,*y_loc,*z_loc;
  bool loc_calculated;

  bool normalized;
  bool rms_calculated;
};



#endif //_DENSITYMAP_H
