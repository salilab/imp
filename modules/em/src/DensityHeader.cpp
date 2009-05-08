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
  if (top_calculated_ && !force)
    return;

  xtop_ = xorigin_ + Objectpixelsize*nx;
  ytop_ = yorigin_ + Objectpixelsize*ny;
  ztop_ = zorigin_ + Objectpixelsize*nz;
  top_calculated_ = true;
}

void DensityHeader::update_map_dimensions(int nnx,int nny,int nnz) {
  nx=nnx;
  ny=nny;
  nz=nnz;
  mx = nx;
  my = ny;
  mz = nz;
  top_calculated_ = false;
  compute_xyz_top();
  xlen=nx*Objectpixelsize;
  ylen=ny*Objectpixelsize;
  zlen=nz*Objectpixelsize;
}

IMPEM_END_NAMESPACE
