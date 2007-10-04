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


class DensityMap:   public Map3D<float>
{
public:
  //---
  // Creates a density map from the templateP with the given resolution and spacing. 
  DensityMap(){
    loc_calculated = false;
  }

  void Read(const char *filename, MapReaderWriter &reader);
  void Write(const char *filename,MapReaderWriter &writer);
  void setResolution(const float &res_) { header.resolution = res_;}


  // data managment
  void ResetData() {
    for (int i=0;i<header.nx*header.ny*header.nz;i++) {
      data[i]=0.0;
    }
  }
  float calcRMS();


  // voxel inspection
  float voxel2loc(const int &index,int dim);

  // header inspection 
  const DensityHeader &get_header()const {return header;}




protected:
  void calc_all_voxel2loc(); // TODO - which is a better desing - have it public or call it from voxel2loc?

  DensityHeader header;
  float *x_loc;
  float *y_loc;
  float *z_loc;
  bool loc_calculated;
};



#endif //_DENSITYMAP_H
