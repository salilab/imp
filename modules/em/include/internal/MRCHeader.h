/**
 *  \file MRCHeader.h
 *  \brief Header information for an MRC file.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_INTERNAL_MRC_HEADER_H
#define IMPEM_INTERNAL_MRC_HEADER_H

#include <IMP/em/em_config.h>
#include "../DensityHeader.h"

IMPEM_BEGIN_INTERNAL_NAMESPACE

#define IMP_MRC_LABEL_SIZE 80
#define IMP_MRC_USER 25
#define IMP_MRC_NUM_LABELS 10

//! Class to deal with the header of MRC files
class IMPEMEXPORT MRCHeader {
 public:
  //! map size (x dimension)
  int nx;
  //! map size (y dimension)
  int ny;
  //! map size (z dimension)
  int nz;
  //! Image mode
  /**
   * 0 image : signed 8-bit bytes range -128 to 127.
   * 1 image : 16-bit halfwords.
   * 2 image : 32-bit reals.
   * 3 transform : complex 16-bit integers
   * 4 transform : complex 32-bit reals
  **/
  int mode;
  //! number of first columns in map (default = 0)
  int nxstart, nystart, nzstart;
  //! Intervals along dimension x
  int mx;
  //! Intervals along dimension y
  int my;
  //! Intervals along dimension z
  int mz;
  //! Cell dimension (angstroms) for x
  float xlen;
  //! Cell dimension (angstroms) for y
  float ylen;
  //! Cell dimension (angstroms) for z
  float zlen;
  //! Cell angle (degrees) for x
  float alpha;
  //! Cell angle (degrees) for y
  float beta;
  //! Cell angle (degrees) for z
  float gamma;
  //! Axes corresponding to columns (mapc), rows (mapr) and sections (maps)
  //!    (1,2,3 for x,y,z)
  int mapc, mapr, maps;
  //! Minimum density value
  float dmin;
  //! Maximum density value
  float dmax;
  //! Mean density value
  float dmean;
  //! Sapce group number 0 or 1 (default 0)
  int ispg;
  int nsymbt;
  //! Number of bytes used for symmetry data (0 or 80)
  int user[IMP_MRC_USER];
  //! extra space used for anything - 0 by default
  //! Map origin used for transforms (x dimension)
  float xorigin;
  //! Map origin used for transforms (y dimension)
  float yorigin;
  //! Map origin used for transforms (z dimension)
  float zorigin;
  //! character string 'MAP ' to identify file type
  char map[4];
  //! machine stamp (0x11110000 bigendian, 0x44440000 little)
  int machinestamp;
  //! Standard deviation of map from mean density
  float rms;
  //! Number of labels being used
  int nlabl;
  //! text labels
  char labels[IMP_MRC_NUM_LABELS][IMP_MRC_LABEL_SIZE];

 public:
  //! Converter from MRCHeader to DensityHeader
  void FromDensityHeader(const DensityHeader &h);
  //! Converter from DensityHeader to MRCHeader
  void ToDensityHeader(DensityHeader &h);
  //! Outputs coordinates delimited by single space.
  friend std::ostream &operator<<(std::ostream &s, const MRCHeader &v) {
    s << "nx: " << v.nx << " ny: " << v.ny << " nz: " << v.nz << std::endl;
    s << "mode: " << v.mode << std::endl;
    s << "nxstart: " << v.nxstart << " nystart: " << v.nystart
      << " nzstart: " << v.nzstart << std::endl;
    s << "mx: " << v.mx << " my:" << v.my << " mz: " << v.mz << std::endl;
    s << "xlen: " << v.xlen << " ylen: " << v.ylen << " zlen: " << v.zlen
      << std::endl;
    s << "alpha : " << v.alpha << " beta: " << v.beta << " gamma: " << v.gamma
      << std::endl;
    s << "mapc : " << v.mapc << " mapr: " << v.mapr << " maps: " << v.maps
      << std::endl;
    s << "dmin: " << v.dmin << " dmax: " << v.dmax << " dmean: " << v.dmean
      << std::endl;
    s << "ispg: " << v.ispg << std::endl;
    s << "nsymbt: " << v.nsymbt << std::endl;
    s << "user: " << v.user << std::endl;
    s << "xorigin: " << v.xorigin << " yorigin: " << v.yorigin
      << " zorigin: " << v.zorigin << std::endl;
    s << "map: " << v.map << std::endl;
    s << "machinestamp: " << v.machinestamp << std::endl;
    s << "rms: " << v.rms << std::endl;
    s << "nlabl: " << v.nlabl << std::endl;
    s << "labels : " << v.labels << std::endl;
    for (int i = 0; i < v.nlabl; i++)
      s << "labels[" << i << "] = ->" << v.labels[i] << "<-" << std::endl;
    return s;
  }
};

IMPEM_END_INTERNAL_NAMESPACE

#endif /* IMPEM_INTERNAL_MRC_HEADER_H */
