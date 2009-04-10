/**
 *  \file SpiderHeader.h
 *  \brief Header for Spider and Xmipp formats
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_SPIDER_HEADER_H
#define IMPEM_SPIDER_HEADER_H

#include "config.h"

IMPEM_BEGIN_NAMESPACE

//! Header for Spider images. IMP-EM is designed to be compatible with it
struct SpiderHeader {
//! Number of slices (=1 for images)
  float fNslice;
  //! Number of rows
  float fNrow;
  //! Total number of records
  float fNrec;
  //! Auxiliary number used to compute the number of records
  float fNlabel;
  //! FILE TYPE SPECIFIER.
  /**
   * +3 FOR A 3-D FILE  (FLOAT)
   * +1 FOR A 2-D IMAGE (FLOAT)
   * -1 FOR A 2-D FOURIER TRANSFORM
   * -3 FOR A 3-D FOURIER TRANSFORM
   * -5 FOR A NEW 2-D FOURIER TRANSFORM
   * -7 FOR A NEW 3-D FOURIER TRANSFORM
   * +8 FOR A 2-D EIGHT BIT IMAGE FILE
   * +9 FOR A 2-D INT IMAGE FILE
   * 10 FOR A 3-D INT IMAGE FILE
   * 11 FOR A 2-D EIGHT BIT COLOR IMAGE FILE
   **/
  float fIform;
  //! MAXIMUM/MINIMUM FLAG.
  /** IS SET AT 0 WHEN THE FILE IS CREATED, AND AT 1 WHEN THE MAXIMUM AND
   * MINIMUM HAVE BEEN COMPUTED, AND HAVE BEEN STORED
   * INTO THIS LABEL RECORD (SEE FOLLOWING WORDS)
   */
  float fImami;
  //! maximum value in the image
  float fFmax;
  //! minimum value in the image
  float fFmin;
  //! average value
  float fAv;
  //! Standard deviation. If -1 it means that it has no been computed
  float fSig;
  //! FLAG INDICATING IF THE HISTOGRAM HAS BE COMPUTED. NOT USED IN 3D FILES!
  float fIhist;
  //! Number of columns
  float fNcol;
  //! NUMBER OF LABEL RECORDS IN FILE HEADER
  float fLabrec;
  //! FLAG THAT TILT ANGLES HAVE BEEN FILLED
  float fIangle;
  //! 1st Euler rotation angle (Rot) (ZYZ convention)
  float fPhi;
  //! 2nd Euler rotation angle (Tilt) (ZYZ convention)
  float fTheta;
  //! 3rd Euler rotation angle (Psi) (ZYZ convention)
  float fPsi;
  //! X TRANSLATION
  float fXoff;
  //! Y TRANSLATION
  float fYoff;
  //! Z TRANSLATION
  float fZoff;
  //! SCALE
  float fScale;
  //! Total number of bytes in the header
  float fLabbyt;
  //! RECORD LENGTH IN BYTES
  float fLenbyt;
  //! this is a Spider incongruence. 24 bytes without meaning
  char  fNothing[24];
  //! THAT ANGLES ARE SET.
  /**
   * 1 = ONE ADDITIONAL
   * ROTATION IS PRESENT, 2 = ADDITIONAL ROTATION
   * THAT PRECEEDS THE ROTATION THAT WAS STORED IN
   * 15 FOR DETAILS SEE MANUAL CHAPTER VOCEUL.MAN
   */
  float fFlag;
  float fPhi1;
  float fTheta1;
  float fPsi1;
  float fPhi2;
  float fTheta2;
  float fPsi2;
  double fGeo_matrix[3][3]; //! x9 = 72 bytes: Geometric info
  float fAngle1; //! angle info
  float fr1;
  float fr2; //! lift up cosine mask parameters
  //! For Radon transforms
  float RTflag; //! 1=RT, 2=FFT(RT)
  float Astart;
  float Aend;
  float Ainc;
  float Rsigma; //! 4*7 = 28 bytes
  float Tstart;
  float Tend;
  float Tinc; //! 4*3 = 12, 12+28 = 40B
  //! For Max-Likelihood refinement (Xmipp compatibility)
  float Weight;
  //! 0= no flipping , 1= flipping  (Xmipp compatibility)
  float Flip;
  /****** beginning of IMP additions *******/
  //! Size in Angstroms of a pixel in the image
  float object_pixel_size;

  //! Size of IMP additions in bytes = 4
  /****** end of IMP additions *******/
  //! Empty field in the SPIDER header format.
  /**
   * Originally is 700 bytes long, but additional information has been added:
   * empty 700-76-40=624-40-8-4(= IMP additions)= 572 bytes
   */
  char empty[572];
  //! date
  char szIDat[12];
  //! time of creation
  char szITim[8];
  //! Title
  char szITit[160];
};


IMPEM_END_NAMESPACE

#endif /* IMPEM_SPIDER_HEADER_H */
