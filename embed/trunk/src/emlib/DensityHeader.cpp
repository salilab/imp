#include "DensityHeader.h"

void DensityHeader::compute_xyz_top()
{
  if (top_calculated)
    return;

  xtop=xorigin+Objectpixelsize*nx;
  ytop=yorigin+Objectpixelsize*ny;
  ztop=zorigin+Objectpixelsize*nz;
  top_calculated = true;
  }

void DensityHeader::update_map_dimensions(int nnx,int nny,int nnz) {
  nx=nnx;
  ny=nny;
  nz=nnz;
  mx = nxstart + nx - 1; 
  my = nystart + ny - 1; 
  mz = nzstart + nz - 1;
  top_calculated=false;
  compute_xyz_top();
  xlen=nx*Objectpixelsize;
  ylen=ny*Objectpixelsize;
  zlen=nz*Objectpixelsize;
}
