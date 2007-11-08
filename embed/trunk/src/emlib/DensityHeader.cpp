#include "DensityHeader.h"
void DensityHeader::compute_xyz_top() {
  if (top_calculated)
    return;
  
  xtop=xorigin+Objectpixelsize*nx;
  ytop=yorigin+Objectpixelsize*ny;
  ztop=zorigin+Objectpixelsize*nz;
  top_calculated = true;
  }
