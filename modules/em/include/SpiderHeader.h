/**
 *  \file SpiderHeader.h
 *  \brief Header for Spider and Xmipp formats
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_SPIDER_HEADER_H
#define IMPEM_SPIDER_HEADER_H

IMPEM_BEGIN_NAMESPACE

//! Header for Spider images. IMP-EM is designed to be compatible with it
typedef struct {
  float fNslice; // Number of slices (=1 for images)
  float fNrow;   // Number of rows
  float fNrec; // Total number of records
  float fNlabel; // Auxiliary number used to compute the number of records
  float fIform; // FILE TYPE SPECIFIER.
  // +3 FOR A 3-D FILE  (FLOAT)
  // +1 FOR A 2-D IMAGE (FLOAT)
  // -1 FOR A 2-D FOURIER TRANSFORM
  // -3 FOR A 3-D FOURIER TRANSFORM
  // -5 FOR A NEW 2-D FOURIER TRANSFORM
  // -7 FOR A NEW 3-D FOURIER TRANSFORM
  // +8 FOR A 2-D EIGHT BIT IMAGE FILE
  // +9 FOR A 2-D INT IMAGE FILE
  // 10 FOR A 3-D INT IMAGE FILE
  // 11 FOR A 2-D EIGHT BIT COLOR IMAGE FILE
  float fImami; // MAXIMUM/MINIMUM FLAG. IS SET AT 0 WHEN THE
  // FILE IS CREATED, AND AT 1 WHEN THE MAXIMUM AND
  // MINIMUM HAVE BEEN COMPUTED, AND HAVE BEEN STORED
  // INTO THIS LABEL RECORD (SEE FOLLOWING WORDS)
  float fFmax; // maximum value in the image
  float fFmin; // minimum value in the image
  float fAv; // average value
  float fSig; // Standard deviation. If -1 it means that it has no been
              // computed.
  float fIhist; // FLAG INDICATING IF THE HISTOGRAM HAS BE
  // COMPUTED. NOT USED IN 3D FILES!
  float fNcol; // Number of columns
  float fLabrec; // NUMBER OF LABEL RECORDS IN FILE HEADER
  float fIangle; // FLAG THAT TILT ANGLES HAVE BEEN FILLED
  float fPhi;   // 1st Euler rotation angle (Rot) (ZYZ convention)
  float fTheta; // 2nd Euler rotation angle (Tilt) (ZYZ convention)
  float fPsi;   // 3rd Euler rotation angle (Psi) (ZYZ convention)
  float fXoff; // X TRANSLATION
  float fYoff; // Y TRANSLATION
  float fZoff; // Z TRANSLATION
  float fScale; // SCALE
  float fLabbyt; // Total number of bytes in the header
  float fLenbyt; // RECORD LENGTH IN BYTES
  char  fNothing[24]; // this is a spider incongruence
  float fFlag; // THAT ANGLES ARE SET. 1 = ONE ADDITIONAL
  // ROTATION IS PRESENT, 2 = ADDITIONAL ROTATION
  // THAT PRECEEDS THE ROTATION THAT WAS STORED IN
  // 15 FOR DETAILS SEE MANUAL CHAPTER VOCEUL.MAN
  float fPhi1;
  float fTheta1;
  float fPsi1;
  float fPhi2;
  float fTheta2;
  float fPsi2;
  double fGeo_matrix[3][3]; // x9 = 72 bytes: Geometric info
  float fAngle1; // angle info
  float fr1;
  float fr2; // lift up cosine mask parameters
  /** Fraga 23/05/97  For Radon transforms **/
  float RTflag; // 1=RT, 2=FFT(RT)
  float Astart;
  float Aend;
  float Ainc;
  float Rsigma; // 4*7 = 28 bytes
  float Tstart;
  float Tend;
  float Tinc; // 4*3 = 12, 12+28 = 40B
  float Weight; // For Max-Likelihood refinement (Xmipp compatibility)
  float Flip; // 0= no flipping , 1= flipping  (Xmipp compatibility)
  /****** beginning of IMP additions *******/
  float object_pixel_size;

  // Size of IMP additions in bytes = 4
  /****** end of IMP additions *******/
  // Empty field in the SPIDER header format
  // Originally is 700 bytes long, but additional information has been added:
  // empty 700-76-40=624-40-8-4(= IMP additions)= 572 bytes
  char empty[572];
  char szIDat[12]; // date
  char szITim[8]; // time of creation
  char szITit[160]; // Title
} SpiderHeader;


IMPEM_END_NAMESPACE

#endif /* IMPEM_SPIDER_HEADER_H */
