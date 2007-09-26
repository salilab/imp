#include "Map3D.h"

#ifdef INCLUDE_SHEILD

template <class dataItemT>
Map3D<dataItemT>::Map3D(int * extent_,int *orig_, float *voxelsize_){
  header = Map3DHeader();
  AllocateData(extent_);
  for(int i=0;i<3;i++) {
    header.set_orig(i,orig_[i]);
    header.set_voxelsize(i,voxelsize_[i]);
  }
}
template <class dataItemT>
Map3D<dataItemT>::Map3D(const Map3D &copy_map) {
  header = Map3DHeader(copy_map);
  allocate_1d_array_range(header.get_extent(2),header.get_extent(1),header.get_extent(0));
}


template <class dataItemT>
Map3D<dataItemT>::~Map3D() {
  free_1d_array_range(data);
}

// template <class dataItemT>
// bool Map3D<dataItemT>::IsValid(const VoxelInd &v) const{
//   for(int i=0;i<3;i++) {
//     if (v[i]<0) return false;
//     if (v[i]>=extent[i]) return false;
//   }
//   return true;
// }



template <class dataItemT>
const dataItemT & Map3D<dataItemT>::get_voxeldata (int i, int j,int k) const {

  // TODO - add validation test

  return data[threeD2oneD_index(i,j,k)];
}



template <class dataItemT>
void Map3D<dataItemT>::setData(int i,int j,int k, const dataItemT d) {
  if ((i<0) || (i>header.get_extent(0)-1) || (j<0) || (j>header.get_extent(1)-1) || (k<0) || (k>header.get_extent(2)-1)) {
    cout << " Map3D::setData is out of range: " << i << "  " << j << "  " << k << " and the extent is " << header.get_extent(0) << "  " << header.get_extent(1) << "  " << header.get_extent(2) << endl;
    //todo - raise error
  }
  data[threeD2oneD_index(i,j,k)] = d;
}


//  allocate a float 3d array with rage[1..z][l..y][l..x]
template <class dataItemT>
dataItemT * Map3D<dataItemT>::allocate_1d_array_range(int zdim,int ydim,int xdim) {
  int i,j;

  float *t;
  t = (dataItemT *)malloc((zdim*ydim*xdim)*sizeof(dataItemT));
  if (!data) 
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


// template <class dataItemT>
// void Map3D<dataItemT>::PrintXplorFormat(ostream& s) const{
//   s <<endl << "       2"<<endl << "REMARKS file name = ??? " << endl << "REMARKS Date ...... created by em lib " << endl;
//   s.setf(ios::right, ios::adjustfield);
//   s.width(8);
//   s << setw(8)<<grid[0]<<
//     setw(8)<<floor(translateGrid[0]/spacing[0])+1<<
//     setw(8)<<floor(translateGrid[0]/spacing[0])+1+extent[0]-1<<
//     setw(8)<<grid[1]<<
//     setw(8)<<floor(translateGrid[1]/spacing[1])+1<<
//     setw(8)<<floor(translateGrid[1]/spacing[1])+1 + extent[1]-1<<
//     setw(8)<<grid[2]<<
//     setw(8)<<floor(translateGrid[2]/spacing[2])+1<<
//     setw(8)<<floor(translateGrid[2]/spacing[2])+1+extent[2]-1<<endl;
    
//   s<< scientific << setprecision(5)<<setw(12)<<cellSize[0];
//   s<< scientific << setprecision(5)<<setw(12)<<cellSize[1];
//   s<< scientific << setprecision(5)<<setw(12)<<cellSize[2];
//   s<< scientific << setprecision(5)<<setw(12)<<cellAngle[0];
//   s<< scientific << setprecision(5)<<setw(12)<<cellAngle[1];
//   s<< scientific << setprecision(5)<<setw(12)<<cellAngle[2]<<endl;
//   s << "ZYX" << endl; // X is the slowest
//   int counter = 0;
//   cout << " in print2file " << endl;
//   for(int k=0;k<extent[2];k++)
//     { 
//       if (counter != 0){
// 	  s << endl;
// 	  counter=0;
// 	}
//         s<<setw(8)<<k<<endl;
// 	for(int j=0;j<extent[1];j++) {
// 	  for(int i=0;i<extent[0];i++) {
// 	    //	    s<< scientific << setprecision(5)<<setw(12)<<data[i][j][k];
// 	    s<< scientific << setprecision(5)<<setw(12)<<getPrintValue(i,j,k);
// 	    counter++;
// 	    if (counter == 6) {
// 	      counter = 0;
// 	      s << endl;
// 	    }
// 	  }
// 	}
//     }
//   s<<endl << "  -9999" << endl;
// }


// template <class dataItemT>
// Vector3 Map3D<dataItemT>::getLocation(const VoxelInd &v) const{
//   return Vector3(translateGrid[0]+spacing[0]*(v[0]+0.5),
// 		 translateGrid[1]+spacing[1]*(v[1]+0.5),
// 		 translateGrid[2]+spacing[2]*(v[2]+0.5));

// }
// template <class dataItemT>
// VoxelInd Map3D<dataItemT>::getVoxel(const Vector3 &loc,int dummy) const {
//   int v_x,v_y,v_z;
//    v_x = (int)round((1.0*loc[0] - translateGrid[0])/spacing[0]-0.5);
//    v_y = (int)round((1.0*loc[1] - translateGrid[1])/spacing[1]-0.5);
//    v_z = (int)round((1.0*loc[2] - translateGrid[2])/spacing[2]-0.5);
// //  v_x = (int)floor((1.0*loc[0] - translateGrid[0])/spacing[0]);
//   // v_y = (int)floor((1.0*loc[1] - translateGrid[1])/spacing[1]);
//   //v_z = (int)floor((1.0*loc[2] - translateGrid[2])/spacing[2]);

//   return VoxelInd(v_x,v_y,v_z);
// }
// // loc is in absolute coordinates
// template <class dataItemT>
// VoxelInd Map3D<dataItemT>::getVoxel(const Vector3 &loc) const {

//   VoxelInd voxel = getVoxel(loc,4);
//   int v_x = voxel[0];
//   int v_y = voxel[1];
//   int v_z = voxel[2];

//   if(v_x >= extent[0] || v_x <0 ||
//      v_y >= extent[1] || v_y <0 ||
//      v_z >= extent[2] || v_z <0) {
//     cout << "Map3D::getVoxel the location in out of range " << loc << " the translate grid is " << translateGrid << endl << v_x << "  " << v_y << "   "<< v_z << endl;
//     exit(-1);
//   }
//   return voxel;
// }


// template <class dataItemT>
// unsigned long Map3D<dataItemT>::getIndex(const VoxelInd &v) const{
// // this is raw order 
//   return  (unsigned long)v[0]*(unsigned long)extent[2]*(unsigned long)extent[1] + (unsigned long)v[1] * (unsigned long)extent[2] + (unsigned long)v[2]; }




#endif
