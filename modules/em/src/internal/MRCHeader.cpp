/**
 *  \file MRCHeader.cpp
 *  \brief Header information for an MRC file.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/internal/MRCHeader.h>

IMPEM_BEGIN_INTERNAL_NAMESPACE

void MRCHeader::FromDensityHeader(const DensityHeader &h) {
  std::string empty;

  // Assume 1:1 mapping here from DensityMap xyz coordinate system to
  // MRC crs system
  ns = h.get_nz();
  nr = h.get_ny();
  nc = h.get_nx();  // map size
  // mode
  if (h.get_data_type() == 0)  // data type not initialized
    mode = 2;
  if (h.get_data_type() == 1)
    mode = 0;  // 8-bits
  else if (h.get_data_type() == 2)
    mode = 1;  // 16-bits
  else if (h.get_data_type() == 5)
    mode = 2;  // 32-bits

  // number of first columns in map (default = 0)
  ncstart = h.nxstart;
  nrstart = h.nystart;
  nsstart = h.nzstart;

  mx = h.mx;
  my = h.my;
  mz = h.mz;  // Number of intervals along each dimension
  xlen = h.xlen;
  ylen = h.ylen;
  zlen = h.zlen;  // Cell dimensions (angstroms)
  alpha = h.alpha;
  beta = h.beta;
  gamma = h.gamma;  // Cell angles (degrees)
  /* Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
     (1,2,3 for x,y,z) */
  mapc = h.mapc;
  mapr = h.mapr;
  maps = h.maps;
  /* Minimum, maximum and mean density value */
  dmin = h.dmin;
  dmax = h.dmax;
  dmean = h.dmean;
  ispg = h.ispg;      // Sapce group number 0 or 1 (default 0)
  nsymbt = h.nsymbt;  // Number of bytes used for symmetry data (0 or 80)
  // extra space used for anything - 0 by default
  for (int i = 0; i < IMP_MRC_USER; i++) user[i] = h.user[i];
  strcpy(map, "MAP");  // character string 'MAP\0' to identify file type
  // Origin used for transforms
  xorigin = h.get_xorigin();
  yorigin = h.get_yorigin();
  zorigin = h.get_zorigin();
  // machine stamp (0x11110000 bigendian, 0x44440000 little)
  machinestamp = h.machinestamp;
  rms = h.rms;      // RMS deviation of map from mean density
  nlabl = h.nlabl;  // Number of labels being used
  // Fill empty coments with null character
  memset(&labels[0][0], 0, IMP_MRC_NUM_LABELS * IMP_MRC_LABEL_SIZE);
  // Copy comments
  for (int i = 0; i < nlabl; i++) strcpy(labels[i], h.comments[i]);
}

void MRCHeader::convert_crs_to_xyz(int &nx, int &ny, int &nz,
                                   int &nxstart, int &nystart, int &nzstart) {
  int nxyz[3] = {0,0,0};
  nxyz[mapc-1] = nc;
  nxyz[mapr-1] = nr;
  nxyz[maps-1] = ns;
  nx = nxyz[0];
  ny = nxyz[1];
  nz = nxyz[2];

  int nxyzstart[3] = {0,0,0};
  nxyzstart[mapc-1] = ncstart;
  nxyzstart[mapr-1] = nrstart;
  nxyzstart[maps-1] = nsstart;
  nxstart = nxyzstart[0];
  nystart = nxyzstart[1];
  nzstart = nxyzstart[2];
}

void MRCHeader::ToDensityHeader(DensityHeader &h) {
  std::string empty;
  /* Convert MRC column/row/section data to XYZ coordinate system used by
     DensityMap */
  int nx, ny, nz, nxstart, nystart, nzstart;
  convert_crs_to_xyz(nx, ny, nz, nxstart, nystart, nzstart);
  h.update_map_dimensions(nx, ny, nz);
  h.update_cell_dimensions();
  // mode
  if (mode == 0)
    h.set_data_type(1);
  else if (mode == 1)
    h.set_data_type(2);
  else if (mode == 2)
    h.set_data_type(5);
  // number of first columns in map (default = 0)
  h.nxstart = nxstart;
  h.nystart = nystart;
  h.nzstart = nzstart;
  h.mx = mx;
  h.my = my;
  h.mz = mz;  // Number of intervals along each dimension
  h.xlen = xlen;
  h.ylen = ylen;
  h.zlen = zlen;  // Cell dimentions(amgstroms)
  h.alpha = alpha;
  h.beta = beta;
  h.gamma = gamma;  // Cell angles (degrees)
  /* Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
     (1,2,3 for x,y,z) */
  h.mapc = mapc;
  h.mapr = mapr;
  h.maps = maps;
  /* Minimum, maximum and mean density value */
  h.dmin = dmin;
  h.dmax = dmax;
  h.dmean = dmean;
  h.ispg = ispg;      // Sapce group number 0 or 1 (default 0)
  h.nsymbt = nsymbt;  // Number of bytes used for symmetry data (0 or 80)
  // extra space used for anything - 0 by default
  for (int i = 0; i < IMP_MRC_USER; i++) h.user[i] = user[i];
  strcpy(h.map, "MAP");  // character string 'MAP\0' to identify file type
  // Origin used for transforms
  // Modern files are supposed to use xorigin/yorigin/zorigin, but if these
  // are zero use the old nxstart/nystart/nzstart fields instead
  // (although only for non-skewed axes, the common case)
  if (std::abs(xorigin) < 1e-6 && std::abs(yorigin) < 1e-6
      && std::abs(zorigin) < 1e-6 && std::abs(alpha-90.0) < 1e-6
      && std::abs(beta-90.0) < 1e-6 && std::abs(gamma-90.0) < 1e-6) {
    h.set_xorigin(nxstart * xlen/mx);
    h.set_yorigin(nystart * ylen/my);
    h.set_zorigin(nzstart * zlen/mz);
  } else {
    h.set_xorigin(xorigin);
    h.set_yorigin(yorigin);
    h.set_zorigin(zorigin);
  }
  // machine stamp (0x11110000 bigendian, 0x44440000 little)
  h.machinestamp = machinestamp;
  h.rms = rms;      // RMS deviation of map from mean density
  h.nlabl = nlabl;  // Number of labels being used
  // Fill empty coments with null character
  memset(&h.comments[0][0], 0,
         DensityHeader::COMMENT_FIELD_NUM_OF
         * DensityHeader::COMMENT_FIELD_SINGLE_SIZE);
  // Copy comments
  for (int i = 0; i < h.nlabl; i++) {
    strncpy(h.comments[i], labels[i], DensityHeader::COMMENT_FIELD_SINGLE_SIZE);
  }
}

IMPEM_END_INTERNAL_NAMESPACE
