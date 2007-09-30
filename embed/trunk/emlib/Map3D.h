#ifndef _MAP3D_H
#define _MAP3D_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT
  responsibilities: holds and provides access to a 3D grid

*/

#include "Vector3.h"

#include <math.h>
#include <stdlib.h>

using namespace std;


template<class dataItemT> class Map3D;

template <class dataItemT>
class Map3D {
public:

  // constructors

  Map3D() {}
  Map3D(int nx_,int ny_,int nz_,float *voxelsize_);
  //TODO: should the copy cont copy all the data?
  //Map3D(const Map3D &copy_map); // copy constuctor
  ~Map3D();


  // handle the data allocation/release
  int Release(); // free the allocated grid


  int threeD2oneD_index(int x_ind, int y_ind, int z_ind) {
    // TODO: should we add here a validation check
    return x_ind+y_ind*nx+z_ind*nx*ny;
  }

  // given a location - check if it is part of the grid
  //bool IsValid(const Vector3 &loc) const {return IsValid(getVoxel(loc,2));}

   const dataItemT  &get_voxeldata(int i, int j,int k) const;
//   const dataItemT  &getData(const Vector3 &loc) const {return getData(getVoxel(loc));}



  
   void setData(int i,int j,int k, dataItemT);
//     setData(v[0],v[1],v[2],d);
//   }
//   void setData(const Vector3 &loc, dataItemT d) {
//     setData(getVoxel(loc),d);
//   }
 
  // VoxelInd getVoxel(const Vector3 &loc) const ;
//   VoxelInd getVoxel(const Vector3 &loc,int dummy) const ; // this means that you should not exit if out of range




  const dataItemT  & getPrintValue(int i, int j, int k) const { return data[i][j][k]; }


  // TODO - where the transgrid should be?
  //  const Vector3 & getTranslateGrid() const { return translateGrid;}
  //  int setTranslateGrid(const Vector3 _translateGrid)  {translateGrid = _translateGrid;}


  // const dataItemT& operator[](const const_iterator &it) const{
//     return getData(it.getPosition());
//   }
//    dataItemT operator[](const VoxelInd &v_i) const{
//     return getData(v_i);
//   }
//   const Vector3& operator[](unsigned long index) const{
//     ldiv_t dimX=ldiv(index, extent[2]*extent[1]);
    
//     unsigned long i = dimX.quot;

//     ldiv_t dimY=ldiv(dimX.rem, extent[2]);

//     unsigned long j = dimY.quot;
//     unsigned long k = dimY.rem;


//     return getLocation(VoxelInd(i,j,k));
//   }

protected:
  void free_1d_array_range(dataItemT *t);
  dataItemT * allocate_1d_array_range(int zdim,int ydim,int xdim);

  // orientation data
  Vector3 translateGrid; // the left-buttom voxel of the grid is assumed to be located at (0,0,0). The exact location of the grid is found in the XPLOR file. We insert this data to the translateGrid vector

  dataItemT *data; // the order is ZYX (Z-slowest)
  float voxelsize[3];
  int nx,ny,nz;
private:
  bool allocated;
};






#define INCLUDE_SHEILD
#include "Map3D.cc"
#undef INCLUDE_SHEILD


#endif //_MAP3D_H
