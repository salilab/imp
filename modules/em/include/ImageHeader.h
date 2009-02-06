/**
 *  \file ImageHeader.h
 *  \brief Header for EM images
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_IMAGE_HEADER_H
#define IMPEM_IMAGE_HEADER_H

#include "config.h"
#include "endian.h"
#include "IMP/algebra/Matrix2D.h"
#include "IMP/algebra/utility.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

IMPEM_BEGIN_NAMESPACE

// Class to deal with the header of Electron Microscopy images in IMP.
/**
 * (Compatible with Xmipp and Spider formats)
 */
class IMPEMEXPORT ImageHeader
{
public:
  //! Types of initialization of ImageHeader
  typedef enum { IMG_BYTE = 0, IMG_IMPEM  = 1,
                 IMG_INT  = 9, /*IMG_SHORT  = , IMG_UCHAR = ,*/
                 VOL_BYTE = 2,  VOL_IMPEM  = 3,
                 VOL_INT  = 10 , /*VOL_SHORT  = , VOL_UCHAR =, */
                 IMG_FOURIER = -1, VOL_FOURIER = -3
               } img_type;

  //! Constructor. The type of image can be specified
  ImageHeader(img_type im = IMG_IMPEM) {
    clear();
    im_ = im;
  }

  //! Get the type of image
  img_type get_image_type() {
    return im_;
  }

  //! Set the type of image
  /**
   * Available values:
   * IMG_BYTE = 0, IMG_IMPEM  = 1,IMG_INT  = 9, VOL_BYTE = 2,  VOL_IMPEM  = 3,
   * VOL_INT  = 10 , IMG_FOURIER = -1, VOL_FOURIER = -3
   * \note No check is done for correctness of the value
   */
  void set_image_type(img_type im) {
    im_ = im;
  }

  //! Output operator
  friend std::ostream& operator<<(std::ostream& out, const ImageHeader& I) {
    out << "Image type   : ";
    switch ((int) I.header_.fIform) {
    case IMG_BYTE:
      out << "2D Byte image";
      break;
    case IMG_IMPEM:
      out << "2D IMP EM image";
      break;
    case IMG_INT:
      out << "2D INT image";
      break;
    case VOL_BYTE:
      out << "3D Byte volume";
      break;
    case VOL_IMPEM:
      out << "3D IMP EM volume";
      break;
    case VOL_INT:
      out << "3D INT volume";
      break;
    case IMG_FOURIER:
      out << "2D Fourier image";
      break;
    case VOL_FOURIER:
      out << "3D Fourier volume";
      break;
    }
    out << std::endl;
    out << "Reversed     : ";
    if (I.reversed_) {
      out << "TRUE" << std::endl;
    } else {
      out << "FALSE" << std::endl;
    }
    out << "dimensions   : " << I.header_.fNslice << " x "
    << I.header_.fNrow   << " x "  << I.header_.fNcol
    << " (slices x rows x columns) " << std::endl;

    out << "Euler angles (ZYZ convention): " << std::endl;
    out << "  Phi   (rotation around Z axis) = " <<
           I.header_.fPhi << std::endl;
    out << "  Theta (tilt, rotation around new Y axis) = " <<
            I.header_.fTheta << std::endl;
    out << "  Psi   (third rotation around new Z axis) = " <<
            I.header_.fPsi << std::endl;
    out << "Origin Offsets : " << std::endl;
    out << "  Xoff  (origin offset in X-direction) = " <<
            I.header_.fXoff << std::endl;
    out << "  Yoff  (origin offset in Y-direction) = " <<
            I.header_.fYoff << std::endl;
    if (I.header_.fFlag == 1.0f || I.header_.fFlag == 2.0f) {
      out << "  Phi1   = " << I.header_.fPhi1 ;
      out << "  theta1 = " << I.header_.fTheta1 ;
      out << "  Psi1   = " << I.header_.fPsi1 << std::endl;
    }
    if (I.header_.fFlag == 2.0f) {
      out << "  Phi2   = " << I.header_.fPhi2 ;
      out << "  theta2 = " << I.header_.fTheta2 ;
      out << "  Psi2   = " << I.header_.fPsi2 << std::endl;
    }
    out << "Date         : " << I.get_date() << std::endl;
    out << "Time         : " << I.get_time() << std::endl;
    out << "Title        : " << I.get_title() << std::endl;
    out << "Header size  : " << I.get_header_size() << std::endl;
    out << "Weight  : " << I.get_Weight() << std::endl;
    return out;
  }


  //! Prints a reduced set of information (debugging purposes)
  void print_hard(std::ostream& out) const;


  //! Reads the header of a EM image
  // \note reversed is only used in case that the type_check is skipped
  int read(const String filename, bool skip_type_check = false,
           bool force_reversed = false, bool skip_extra_checkings = false);

  //! Reads the header of a EM image from an input file stream
  bool read(std::ifstream& f, bool skip_type_check = false,
            bool force_reversed = false, bool skip_extra_checkings = false);

  //! Writes the header of a EM image
  void write(const String &filename, bool force_reversed = false);

  //! Writes the header of a EM image to an output file stream
  void write(std::ofstream& f, bool force_reversed = false);

  //! Clear header data (set all header to zero )
  void clear();

  //! Sets a consistent header
  void set_header();

  //! Interaction with data
  bool reversed() const {
    return reversed_;
  }

  //! Interaction with data
  bool& reversed() {
    return reversed_;
  }

  //! get header size
  int get_header_size() const {
    return (int) header_.fNcol *(int) header_.fLabrec * sizeof(float);
  }

  //! Get number of slices
  float get_slices()  const {
    return header_.fNslice;
  }

  //! Set number of slices
  void set_slices(float n) {
    header_.fNslice = n;
  }

  //! Get number of rows
  float get_rows() const {
    return header_.fNrow;
  }

  //! Get number of rows
  void set_rows(float n) {
    header_.fNrow = n;
  }

  //! Get number columns
  float get_columns() const {
    return header_.fNcol;
  }

  //! Get number columns
  void set_columns(float n) {
    header_.fNcol = n;
  }

  //! Rotation Angle
  float get_old_rot() const {
    return header_.fAngle1;
  }

  float& get_old_rot() {
    return header_.fAngle1;
  }

  float  get_Scale() const {
    return header_.fScale;
  }

  float& get_Scale() {
    return header_.fScale;
  }

  //! For Maximum-Likelihood refinement (for compatibility: not currently used)
  float get_Flip() const {
    return header_.Flip;
  }

  float& get_Flip() {
    return header_.Flip;
  }

  float get_Weight() const {
    return header_.Weight;
  }

  float& get_Weight() {
    return header_.Weight;
  }

  float get_fNrec() const {
    return header_.fNrec;
  }

  float& get_fNrec() {
    return header_.fNrec;
  }

  float get_fNlabel() const {
    return header_.fNlabel;
  }

  float& get_fNlabel() {
    return header_.fNlabel;
  }

  float get_fIform() const {
    return header_.fIform;
  }

  float& get_fIform() {
    return header_.fIform;
  }

  float get_fImami() const {
    return header_.fImami;
  }

  float& get_fImami() {
    return header_.fImami;
  }

  float get_fFmax() const {
    return header_.fFmax;
  }

  float& get_fFmax() {
    return header_.fFmax;
  }

  float get_fFmin() const {
    return header_.fFmin;
  }

  float& get_fFmin() {
    return header_.fFmin;
  }

  float get_fAv() const {
    return header_.fAv;
  }

  float& get_fAv() {
    return header_.fAv;
  }

  float get_fSig() const {
    return header_.fSig;
  }

  float& get_fSig() {
    return header_.fSig;
  }

  float get_fIhist()  const {
    return header_.fIhist;
  }

  float& get_fIhist() {
    return header_.fIhist;
  }

  float get_fLabrec() const {
    return header_.fLabrec;
  }

  float& get_fLabrec() {
    return header_.fLabrec;
  }

  float get_fIangle() const {
    return header_.fIangle;
  }

  float& get_fIangle() {
    return header_.fIangle;
  }

  float get_xorigin() const {
    return header_.fXoff;
  }

  void set_xorigin(float value) {
    header_.fXoff = value;
  }

  float get_yorigin() const {
    return header_.fYoff;
  }

  void set_yorigin(float value) {
    header_.fYoff = value;
  }

  float get_zorigin() const {
    return header_.fZoff;
  }

  void set_zorigin(float value) {
    header_.fZoff = value;
  }

  float get_object_pixel_size() const {
    return header_.object_pixel_size;
  }

  void set_object_pixel_size(float value) {
    header_.object_pixel_size = value;
  }

  float get_fLabbyt() const {
    return header_.fLabbyt;
  }

  float& get_fLabbyt() {
    return header_.fLabbyt;
  }

  float get_fLenbyt() const {
    return header_.fLenbyt;
  }

  float& get_fLenbyt() {
    return header_.fLenbyt;
  }

  algebra::Matrix2D< double > get_fGeo_matrix();

  // Origin offsets
  void set_origin_offsets(float Xoff, float Yoff);
  void get_origin_offsets(float& Xoff, float& Yoff) const;

  // Euler angles
  void set_euler_angles(float Phi, float Theta, float Psi);
  void set_euler_angles1(float Phi1, float Theta1, float Psi1);
  void set_euler_angles2(float Phi2, float Theta2, float Psi2);




  //! Clears fFlag flag.
  /** The number of triads of Euler angles stored in the header (up to three)
    is stored here. set_euler_angles2 makes fFlag=2, set_euler_angles1 makes
    fFlag=max(fFlag, 1), set_euler_angles does not change fFlag
  */
  void clear_fFlag_flag() {
    header_.fFlag = 0.f;
  }


  template<typename T>
  void get_euler_angles(T& Phi, T& Theta, T& Psi) const {
    Phi = (T) header_.fPhi;
    Theta = (T) header_.fTheta;
    Psi = (T) header_.fPsi;
  }

  template<typename T>
  void get_euler_angles1(T& Phi1, T& Theta1, T& Psi1) const {
    Phi1 = (T) header_.fPhi1;
    Theta1 = (T) header_.fTheta1;
    Psi1 = (T) header_.fPsi1;
  }

  template<typename T>
  void get_euler_angles2(T& Phi2, T& Theta2, T& Psi2) const {
    Phi2 = (T) header_.fPhi2;
    Theta2 = (T) header_.fTheta2;
    Psi2 = (T) header_.fPsi2;
  }

  float& get_Phi() {
    header_.fIangle = 1;
    return header_.fPhi;
  }

  float get_Phi() const {
    return header_.fPhi;
  }

  float& get_Theta() {
    header_.fIangle = 1;
    return header_.fTheta;
  }

  float get_Theta() const {
    return header_.fTheta;
  }

  float& get_Psi() {
    header_.fIangle = 1;
    return header_.fPsi;
  }

  float get_Psi() const {
    return header_.fPsi;
  }

  float& get_Phi1() {
    header_.fFlag = 1.f;
    return header_.fPhi1;
  }

  float get_Phi1() const {
    return header_.fPhi1;
  }

  float& get_Theta1() {
    header_.fFlag = 1.f;
    return header_.fTheta1;
  }

  float get_Theta1() const {
    return header_.fTheta1;
  }

  float& get_Psi1() {
    header_.fFlag = 1.f;
    return header_.fPsi1;
  }

  float get_Psi1() const {
    return header_.fPsi1;
  }

  float& get_Phi2() {
    header_.fFlag = 2.f;
    return header_.fPhi2;
  }

  float get_Phi2() const {
    return header_.fPhi2;
  }

  float& get_Theta2() {
    header_.fFlag = 2.f;
    return header_.fTheta2;
  }

  float get_Theta2() const {
    return header_.fTheta2;
  }

  float& get_Psi2() {
    header_.fFlag = 2.f;
    return header_.fPsi2;
  }

  float get_Psi2() const {
    return header_.fPsi2;
  }

  float is_flag_set(void) {
    return(header_.fFlag);
  }



  // Date and Time
  char* get_date() const;
  char* get_time() const;
  void set_date();
  void set_time();

  //! Set info about the image dimension in the header
  void set_dimensions(float Ydim, float Xdim);
  //! Get info about the image dimension from the header
  void get_dimensions(float &Ydim, float &Xdim) const;


  // Title
  char* get_title() const;

  //! Set title of image in the header
  void set_title(String newName);

private:

//! Header for Spider images. IMP-EM is designed to be compatible with it
  /**
    * Set as protected in order to avoid direct manipulation
    */
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
    char  fNada[24]; // this is a spider incongruence
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
    float Weight; // For Maximum-Likelihood refinement ( not used)
    float Flip; // 0=no flipping operation (false), 1=flipping (true) (not used)
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

  SpiderHeader header_;
  img_type im_;
  bool reversed_;

}; // ImageHeader

IMPEM_END_NAMESPACE

#endif /* IMPEM_IMAGE_HEADER_H */
