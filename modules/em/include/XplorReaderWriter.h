/**
 *  \file XplorReaderWriter.h
 *  \brief Classes to read or write density files in XPLOR format.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_XPLOR_READER_WRITER_H
#define IMPEM_XPLOR_READER_WRITER_H

#include "config.h"
#include "MapReaderWriter.h"
#include "DensityHeader.h"
#include "ErrorHandling.h"
#include <math.h>
#include <iostream>
#include <iomanip>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT XplorHeader
{
public:
  XplorHeader() {}
  XplorHeader(const DensityHeader &m_header) {
    grid[0]=m_header.mx;grid[1]=m_header.my;grid[2]=m_header.mz;
    orig[0]=m_header.nxstart;orig[1]=m_header.nystart;orig[2]=m_header.nzstart;
    extent[0]=m_header.nx;extent[1]=m_header.ny;extent[2]=m_header.nz;
    cellsize[0] = m_header.xlen;
    cellsize[1] = m_header.ylen;
    cellsize[2] = m_header.zlen;
    cellangle[0] = m_header.alpha;
    cellangle[1] = m_header.beta;
    cellangle[2] = m_header.gamma;
    voxelsize[0] = m_header.xlen/m_header.mx;
    voxelsize[1] = m_header.ylen/m_header.my;
    voxelsize[2] = m_header.zlen/m_header.mz;
    translateGrid[0] = m_header.get_xorigin();
    translateGrid[1] = m_header.get_yorigin();
    translateGrid[2] = m_header.get_zorigin();
  }
  void GenerateCommonHeader(DensityHeader &m_header) {
    m_header.mx=grid[0];m_header.my=grid[1];m_header.mz=grid[2];
    m_header.nxstart=orig[0];m_header.nystart=orig[1];m_header.nzstart=orig[2];
    m_header.nx=extent[0];m_header.ny=extent[1];m_header.nz=extent[2];
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


class IMPEMEXPORT XplorReaderWriter : public MapReaderWriter
{
public:
  void Read(const char *filename, float **data, DensityHeader &header);
  void Write(const char *filename, const float *data,
             const DensityHeader &header );
protected:
  int ReadHeader(std::ifstream & XPLORstream, XplorHeader &header);
  int ReadMap(std::ifstream &XPLORstream, float *data, XplorHeader &header);

};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_XPLOR_READER_WRITER_H */
