#include "Map3D.h"

#ifdef INCLUDE_SHEILD

template <class dataItemT>
Map3D<dataItemT>::Map3D(int nx_,int ny_,int nz_,float *voxelsize_) : nnx(nx_),nny(ny_),nnz(nz_){
  for(int i=0;i<3;i++) {
    voxelsize[i]=voxelsize_[i];
  }
  allocate_1d_array_range(nnz,nny,nnx);

}

// template <class dataItemT>
// Map3D<dataItemT>::Map3D(const Map3D &copy_map) {
//   allocate_1d_array_range(header.get_extent(2),header.get_extent(1),header.get_extent(0));
// }


template <class dataItemT>
Map3D<dataItemT>::~Map3D() {
  free_1d_array_range(data_);
}


template <class dataItemT>
const dataItemT  Map3D<dataItemT>::get_voxeldata (int i, int j,int k) const {

  // TODO - add validation test
  dataItemT val = data_[threeD2oneD_index(i,j,k)];
  return val;
}



template <class dataItemT>
void Map3D<dataItemT>::setData(int i,int j,int k, const dataItemT d) {
  if ((i<0) || (i>nnx-1) || (j<0) || (j>nny-1) || (k<0) || (k>nnz-1)) {
    cout << " Map3D::setData is out of range: " << i << "  " << j << "  " << k << " and the extent is " << nnx << "  " << nny << "  " << nnz << endl;
    //todo - raise error
  }
  data_[threeD2oneD_index(i,j,k)] = d;
}


//  allocate a float 3d array with rage[1..z][l..y][l..x]
template <class dataItemT>
dataItemT * Map3D<dataItemT>::allocate_1d_array_range(int zdim,int ydim,int xdim) {
  float *t;
  t = (dataItemT *)malloc((zdim*ydim*xdim)*sizeof(dataItemT));
  if (!data_) 
    {cout <<" Map3D<dataItemT>::AllocateData allocation failure for size " << (zdim*ydim*xdim)*sizeof(dataItemT **) << endl;
      exit(-1);
    }
  else {
    allocated = true;
  }
   return t;

}


// free allocated 1d array with rage[l..z][l..y][l..x] allocated by allocate_1d_array_range 
template <class dataItemT>
void Map3D<dataItemT>::free_1d_array_range(dataItemT *t){
  if (allocated){
    delete t;
    allocated = false;
  }
}


#endif
