/**
 *  \file XplorHeader.h
 *  \brief Header information for an XPLOR file.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_INTERNAL_XPLOR_HEADER_H
#define IMPEM_INTERNAL_XPLOR_HEADER_H

#include <IMP/em/em_config.h>
#include "../DensityHeader.h"

IMPEM_BEGIN_INTERNAL_NAMESPACE

class XplorHeader {
 public:
  XplorHeader() {}
  XplorHeader(const DensityHeader &m_header) {
    grid[0] = m_header.mx;
    grid[1] = m_header.my;
    grid[2] = m_header.mz;
    orig[0] = m_header.nxstart;
    orig[1] = m_header.nystart;
    orig[2] = m_header.nzstart;
    extent[0] = m_header.get_nx();
    extent[1] = m_header.get_ny();
    extent[2] = m_header.get_nz();
    cellsize[0] = m_header.xlen;
    cellsize[1] = m_header.ylen;
    cellsize[2] = m_header.zlen;
    cellangle[0] = m_header.alpha;
    cellangle[1] = m_header.beta;
    cellangle[2] = m_header.gamma;
    voxelsize[0] = m_header.xlen / m_header.mx;
    voxelsize[1] = m_header.ylen / m_header.my;
    voxelsize[2] = m_header.zlen / m_header.mz;
    translateGrid[0] = m_header.get_xorigin();
    translateGrid[1] = m_header.get_yorigin();
    translateGrid[2] = m_header.get_zorigin();
  }
  void GenerateCommonHeader(DensityHeader &m_header) {
    m_header.mx = grid[0];
    m_header.my = grid[1];
    m_header.mz = grid[2];
    m_header.nxstart = orig[0];
    m_header.nystart = orig[1];
    m_header.nzstart = orig[2];
    m_header.update_map_dimensions(extent[0], extent[1], extent[2]);
    m_header.xlen = cellsize[0];
    m_header.ylen = cellsize[1];
    m_header.zlen = cellsize[2];
    m_header.alpha = cellangle[0];
    m_header.beta = cellangle[1];
    m_header.gamma = cellangle[2];
    m_header.set_xorigin(translateGrid[0]);
    m_header.set_yorigin(translateGrid[1]);
    m_header.set_zorigin(translateGrid[2]);
  }

  int grid[3];
  int orig[3];
  int top[3];
  int extent[3];
  float cellsize[3];
  float cellangle[3];
  float voxelsize[3];
  float translateGrid[3];
};

IMPEM_END_INTERNAL_NAMESPACE

#endif /* IMPEM_INTERNAL_XPLOR_HEADER_H */
