#include "VoxelInd.h"
VoxelInd::VoxelInd(int _i,int _j,int _k){
  v[0]=_i;
  v[1]=_j;
  v[2]=_k;
}

VoxelInd::VoxelInd() {
  v[0]=0;
  v[1]=0;
  v[2]=0;
}

VoxelInd::VoxelInd(const VoxelInd &_v) {
  v[0]=_v[0];
  v[1]=_v[1];
  v[2]=_v[2];
}
