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
class Map3D
{
public:

  //! constructors
  Map3D() {}
  Map3D(int nx_,int ny_,int nz_,float *voxelsize_);

  Map3D(const Map3D &copy_map);
  ~Map3D();


  int threeD2oneD_index(int x_ind, int y_ind, int z_ind) const {
    // TODO: should we add here a validation check
    return x_ind+y_ind*nnx+z_ind*nnx*nny;
  }

  const dataItemT get_voxeldata(int i, int j, int k) const;

  void setData(int i,int j,int k, dataItemT);
  const dataItemT* data() const {return data_;}

protected:
  void free_1d_array_range(dataItemT *t);
  dataItemT * allocate_1d_array_range(int zdim,int ydim,int xdim);

  // orientation data
  // the left-buttom voxel of the grid is assumed to be located at (0,0,0).
  // The exact location of the grid is found in the XPLOR file. We insert
  // this data to the translateGrid vector
  Vector3 translateGrid;

  dataItemT *data_; // the order is ZYX (Z-slowest)
  float voxelsize[3];
  int nnx,nny,nnz;
private:
  bool allocated;
};






#define INCLUDE_SHEILD
#include "Map3D.cpp"
#undef INCLUDE_SHEILD


#endif //_MAP3D_H
