#ifndef _VOXELIND_H
#define _VOXELIND_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)

  OVERVIEW TEXT

  responsibilities: holds and manage a voxel index

*/

#include <iostream>
#include <iomanip>

class VoxelInd {
public:
  VoxelInd(int _i,int _j,int _k);
  VoxelInd(const VoxelInd &_v);
  VoxelInd();
  int operator[](unsigned short coord) const{
    return v[coord];
  }

  void update(unsigned short coord,int value){
    v[coord] = value;
  }


  VoxelInd operator-(const VoxelInd &_v) const{
    VoxelInd res_ind;

    for(int i=0;i<3;i++) {
      res_ind.update(i,v[i]-_v[i]);
    }
    return res_ind;
  }
  VoxelInd operator+(const VoxelInd &_v) const{
    VoxelInd res_ind;

    for(int i=0;i<3;i++) {
      res_ind.update(i,v[i]+_v[i]);
    }
    return res_ind;
  }

  bool operator==(const VoxelInd &_v) const{ 
    if ((v[0] == _v[0]) && (v[1] == _v[1]) && (v[2] == _v[2]))
      return true;
    return false;
  }
  bool operator<(const VoxelInd &v2) const{
    int max_grid_size = 256;
    long v1_index = v[0]*max_grid_size*max_grid_size + v[1]*max_grid_size+ v[2];
    long v2_index = v2[0]*max_grid_size*max_grid_size
                     + v2[1]*max_grid_size+ v2[2];
    if (v1_index<v2_index)
      return true;
    return false;
  }
  friend ostream& operator<<(ostream& s, const VoxelInd &v) {
    return s << v[0] << ' ' << v[1] << ' ' << v[2];
  }



private:
  int v[3]; // the oder is ZYX
};
#endif //_VOXELIND_H
