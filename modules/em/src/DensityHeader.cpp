/**
 *  \file DensityHeader.cpp
 *  \brief Metadata for a density file.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/em/DensityHeader.h>

IMPEM_BEGIN_NAMESPACE

void DensityHeader::compute_xyz_top(bool force)
{
  if ((top_calculated) && !force)
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
  mx = nx;
  my = ny;
  mz = nz;
  top_calculated=false;
  compute_xyz_top();
  xlen=nx*Objectpixelsize;
  ylen=ny*Objectpixelsize;
  zlen=nz*Objectpixelsize;
}

IMPEM_END_NAMESPACE
