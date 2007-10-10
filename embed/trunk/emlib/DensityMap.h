#ifndef _DENSITYMAP_H
#define _DENSITYMAP_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@post.tau.ac.il)

  OVERVIEW TEXT

*/




#include "Map3D.h"
#include "DensityHeader.h"
#include "Vector3.h"
#include <iostream>
#include <iomanip>
#include "MapReaderWriter.h"
using std::istream;
using std::ios;

typedef float real;


class DensityMap:   public Map3D<real>
{
public:
  //---
  // Creates a density map from the templateP with the given resolution and spacing. 
  DensityMap();


  void CreateVoidMap(const int &nx,const int &ny,const int &nz);
  void Read(const char *filename, MapReaderWriter &reader);
  void Write(const char *filename,MapReaderWriter &writer);
  void setResolution(const float &res_) { header_.resolution = res_;}


  // data managment
  void ResetData() {
    for (int i=0;i<header_.nx*header_.ny*header_.nz;i++) {
      data_[i]=0.0;
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
  const DensityHeader &header()const {return header_;}
  DensityHeader &header_writable() {return header_;}
  
  float* x_loc() const {return x_loc_;}
  float* y_loc() const {return y_loc_;}
  float* z_loc() const {return z_loc_;}



protected:

  /*calculate the coordinates that correspond to all voxels.
    can be precomputed to make corr faster.  */
  void calc_all_voxel2loc(); // TODO - which is a better desing - have it public or call it from voxel2loc?

  DensityHeader header_;
  float *x_loc_;
  float *y_loc_;
  float *z_loc_;
  bool loc_calculated;
  bool normalized;
  bool rms_calculated;
};



#endif //_DENSITYMAP_H
