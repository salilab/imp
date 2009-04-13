/**
 *  \file header_converters.cpp
 *  \brief Functions to convert between ImageHeader and DensityHeader
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#include <IMP/em/header_converters.h>

IMPEM_BEGIN_NAMESPACE

//! Function to transfer the (compatible) information content from ImageHeader
//! to DensityHeader
void  ImageHeader_to_DensityHeader(const ImageHeader& h,DensityHeader& dh) {
  std::string empty;
  // map size
  dh.nz=h.get_slices();
  dh.ny=h.get_rows();
  dh.nx=h.get_columns();
  // mode
  switch ((int) h.get_fIform()) {
  case ImageHeader::IMG_BYTE:
    dh.data_type=1;
    break;
  case ImageHeader::IMG_IMPEM:
    dh.data_type=5;
    break;
  case ImageHeader::IMG_INT:
    dh.data_type=2;
    break;
  case ImageHeader::VOL_BYTE:
    dh.data_type=1;
    break;
  case ImageHeader::VOL_IMPEM:
    dh.data_type=5;
    break;
  case ImageHeader::VOL_INT:
    dh.data_type=2;
    break;
  }

  // number of first columns in map (default = 0)
  dh.nxstart=0 ; dh.nystart=0 ; dh.nzstart=0;
  // Number of intervals along each dimension. In DensityHeader it is assumed
  // to be equal to the size of the map
  dh.mx=dh.nx;  dh.my=dh.ny;  dh.mz=dh.nz;
  // Cell dimensions (angstroms)
  dh.xlen=h.get_xorigin()+h.get_object_pixel_size()*h.get_columns();
  dh.zlen=h.get_yorigin()+h.get_object_pixel_size()*h.get_rows();
  dh.ylen=h.get_zorigin()+h.get_object_pixel_size()*h.get_slices();
  // Cell angles (degrees)
  // Spider format does not have these fields. Filed with default (90 deg)
  dh.alpha=90. ; dh.beta=90. ; dh.gamma=90.;
  /* Axes corresponding to columns (mapc), rows (mapr) and slices (maps)
     They are given the default values (1,2,3 for x,y,z) */
  dh.mapc=1; dh.mapr=2; dh.maps=3;
  // Statistical values: Minimum, maximum, mean, standard deviation
  // Only set them if computed
  if((int)h.get_fImami()==1) {
    dh.dmin=h.get_fFmin();
    dh.dmax=h.get_fFmax();
    dh.dmean=h.get_fAv();
    dh.rms=h.get_fSig();
  } else {
    dh.dmin=0;
    dh.dmax=0;
    dh.dmean=0;
    dh.rms=0;
  }
  dh.ispg=0.; // Sapce group number 0 or 1 (default 0)
  dh.nsymbt=0; // Number of bytes used for symmetry data (0 or 80, default 0)
  std::strcpy(dh.map,"MAP \0"); // string 'MAP ' to identify file type
  // Origin used for transforms
  dh.set_xorigin(h.get_xorigin());
  dh.set_yorigin(h.get_yorigin());
  dh.set_zorigin(h.get_zorigin());
  /* CCP4 convention machine stamp: 0x11110000 for big endian, or
    0x44440000 for little endian */
  unsigned char *ch;
  ch = (unsigned char *)&dh.machinestamp;
  if(algebra::is_big_endian()) {
    ch[0] = ch[1] = 0x11; ch[2] = ch[3] = 0;
  } else {
    ch[0] = ch[1] = 0x44; ch[2] = ch[3] = 0;
  }
  // Number of labels being used (default, 0)
  dh.nlabl=0;
  // Reversed image or not
  if(h.get_reversed()==true) {
    if(algebra::is_big_endian()) {
      dh.lswap=0;
    } else {
      dh.lswap=1;
    }
  } else {
    if(algebra::is_big_endian()) {
      dh.lswap=1;
    } else {
      dh.lswap=0;
    }
  }
}

//! Function to transfer the (compatible) information content from DensityHeader
//! to ImageHeader
void  DensityHeader_to_ImageHeader(const DensityHeader& dh,ImageHeader& h) {
  // map size
  h.set_dimensions((float)dh.nz,(float)dh.ny, (float)dh.nx);
  // mode
  switch (dh.data_type) {
  case 1:
    h.set_image_type(ImageHeader::VOL_BYTE);
    break;
  case 2:
    h.set_image_type(ImageHeader::VOL_INT);
    break;
  case 5:
    h.set_image_type(ImageHeader::VOL_IMPEM);
    break;
  }
  // Reversed image?
  if(algebra::is_big_endian()) {
    h.set_reversed(dh.lswap!=1);
  } else {
    h.set_reversed(dh.lswap==1);
  }
  // Origins and pixel size
  h.set_xorigin(dh.get_xorigin());
  h.set_yorigin(dh.get_yorigin());
  h.set_zorigin(dh.get_zorigin());
  h.set_object_pixel_size(dh.Objectpixelsize);
  // Statistical values. There is no field in DensityHeader to guarantee that
  // they are computed or correct, so they are ignored
  h.set_fImami(0.);
  h.set_fFmax(0.);
  h.set_fFmin(0.);
  h.set_fAv(0.);
  h.set_fSig(0.);
  // These fields don't have an equivalent in DensityHeader
  h.set_Scale(1.);
  h.set_fAngle1(0.);
  h.set_Weight(1.);
  h.set_Flip(0.);
  h.set_fNrec(0.);
  h.set_fNlabel(0.);
  h.set_fIhist(0.);
  h.set_fLabrec(0.);
  h.set_fIangle(0.);
  h.set_fLabbyt(0.);
  h.set_fLenbyt(0.);
  h.set_fFlag(0.); // Ignore all the Euler angles for the Spider Image
  // Set time and date
  h.set_date();
  h.set_time();
}

IMPEM_END_NAMESPACE
