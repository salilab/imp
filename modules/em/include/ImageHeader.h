/**
 *  \file ImageHeader.h
 *  \brief Header for EM images. Compatible with Spider and Xmipp formats
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
**/

#ifndef IMPEM_IMAGE_HEADER_H
#define IMPEM_IMAGE_HEADER_H

#include "IMP/em/em_config.h"
#include "IMP/em/SpiderHeader.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Vector2D.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/endian.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

IMPEM_BEGIN_NAMESPACE

// Class to deal with the header of Electron Microsocopy images in IMP.
/**
 * (Compatible with SPIDER and Xmipp formats)
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

  //! Constructor.
  ImageHeader() {
    clear();
    header_.fIform=(float)IMG_IMPEM;
  }

  //! Constructor. The type of image can be specified
  ImageHeader(img_type im) {
    clear();
    header_.fIform=(float)im;
  }

  //! Constructor. The type of image can be specified
  ImageHeader(float im) {
    clear();
    header_.fIform=im;
  }

  //! Get the type of image
  float get_image_type() const {
    return header_.fIform;
  }

  //! Set the type of image
  /**
   * param[in] im type of image. Available values:
   * IMG_BYTE = 0, IMG_IMPEM  = 1,IMG_INT  = 9, VOL_BYTE = 2,  VOL_IMPEM  = 3,
   * VOL_INT  = 10 , IMG_FOURIER = -1, VOL_FOURIER = -3
   * \note No check is done for correctness of the value
   */
  //! Set the type of image
  void set_image_type(img_type im) {
    this->set_image_type((float)im);
  }

  //! Set the type of image
  /**
   * param[in] im type of image. Available values:
   */
  void set_image_type(float im) {
    header_.fIform=im;
  }

  void show(std::ostream& out) const {
    out << "Image type   : ";
    switch ((int) this->header_.fIform) {
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
    if (this->reversed_) {
      out << "TRUE" << std::endl;
    } else {
      out << "FALSE" << std::endl;
    }
    out << "dimensions   : " << this->header_.fNslice << " x "
    << this->header_.fNrow   << " x "  << this->header_.fNcol
    << " (slices x rows x columns) " << std::endl;

    out << "Euler angles (ZYZ convention): " << std::endl;
    out << "  Phi   (rotation around Z axis) = " <<
           this->header_.fPhi << std::endl;
    out << "  Theta (tilt, rotation around new Y axis) = " <<
            this->header_.fTheta << std::endl;
    out << "  Psi   (third rotation around new Z axis) = " <<
            this->header_.fPsi << std::endl;
    out << "Origin Offsets : " << std::endl;
    out << "  Xoff  (origin offset in X-direction) = " <<
            this->header_.fXoff << std::endl;
    out << "  Yoff  (origin offset in Y-direction) = " <<
            this->header_.fYoff << std::endl;
    if (this->header_.fFlag == 1.0f || this->header_.fFlag == 2.0f) {
      out << "  Phi1   = " << this->header_.fPhi1 ;
      out << "  theta1 = " << this->header_.fTheta1 ;
      out << "  Psi1   = " << this->header_.fPsi1 << std::endl;
    }
    if (this->header_.fFlag == 2.0f) {
      out << "  Phi2   = " << this->header_.fPhi2 ;
      out << "  theta2 = " << this->header_.fTheta2 ;
      out << "  Psi2   = " << this->header_.fPsi2 << std::endl;
    }
    out << "Date         : " << this->get_date() << std::endl;
    out << "Time         : " << this->get_time() << std::endl;
    out << "Title        : " << this->get_title() << std::endl;
    out << "Header size  : " << this->get_header_size() << std::endl;
    out << "Weight  : " << this->get_Weight() << std::endl;
  }


  //! Prints a reduced set of information (debugging purposes)
  void print_hard(std::ostream& out) const;

  //! Shows only the projection parameters
  inline void show_projection_params(std::ostream& out) const {
    out << "(Phi,Theta,Psi) = ( " << header_.fPhi << " , " << header_.fTheta
        << " , " << header_.fPsi << " ) " << " (x,y) = ( " << header_.fXoff
        << " , " << header_.fYoff << " ) " <<  std::endl;
  }



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

  //! Clear header data and sets a consistent header
  void clear();

  //! Sets a consistent header
  void set_header();

  //! Interaction with data
  bool get_reversed() const {return reversed_; }
  void set_reversed(bool value) { reversed_=value; }

  //! get header size
  int get_header_size() const {
    return (int) header_.fNcol *(int) header_.fLabrec * sizeof(float);
  }

  float get_number_of_slices()  const { return header_.fNslice; }
  void set_number_of_slices(float n) { header_.fNslice = n; }

  float get_number_of_rows() const { return header_.fNrow; }
  void set_number_of_rows(float n) { header_.fNrow = n; }

  float get_number_of_columns() const { return header_.fNcol; }
  void set_number_of_columns(float n) { header_.fNcol = n; }

  //! get rotation angle. (Xmipp compatibility)
  float get_old_rot() const {
    return header_.fAngle1;
  }

  //! set rotation angle. (Xmipp compatibility)
  void set_old_rot(float value) {
    header_.fAngle1= value;
  }

  //! get rotation angle. (Xmipp compatibility)
  float get_fAngle1() const {
    return header_.fAngle1;
  }

  //! set rotation angle. (Xmipp compatibility)
  void set_fAngle1(float value) {
    header_.fAngle1= value;
  }

  float  get_Scale() const {
    return header_.fScale;
  }

  void set_Scale(float value) {
    header_.fScale=value;
  }

  /* For Maximum-Likelihood refinement (Xmipp compatibility:
  not currently used)
  */

  float get_Flip() const { return header_.Flip; }

  /* For Maximum-Likelihood refinement (Xmipp compatibility:
  not currently used)
  */
  void set_Flip(float value) { header_.Flip=value; }

  float get_Weight() const { return header_.Weight; }
  void set_Weight(float value) { header_.Weight=value; }

  float get_fNrec() const { return header_.fNrec; }
  void set_fNrec(float value) { header_.fNrec = value; }

  float get_fNlabel() const { return header_.fNlabel; }
  void set_fNlabel(float value) { header_.fNlabel=value; }

  float get_fIform() const { return header_.fIform; }
  void set_fIform(float value) { header_.fIform = value; }

  float get_fImami() const { return header_.fImami; }
  void set_fImami(float value) { header_.fImami=value; }

  float get_fFmax() const { return header_.fFmax; }
  void set_fFmax(float value) { header_.fFmax= value; }

  float get_fFmin() const { return header_.fFmin; }
  void set_fFmin(float value) { header_.fFmin=value; }

  float get_fAv() const { return header_.fAv; }
  void set_fAv(float value) { header_.fAv=value; }

  float get_fSig() const { return header_.fSig; }
  void set_fSig(float value) { header_.fSig=value; }

  float get_fIhist()  const { return header_.fIhist; }
  void set_fIhist(float value) { header_.fIhist=value; }

  float get_fLabrec() const { return header_.fLabrec; }
  void set_fLabrec(float value) { header_.fLabrec=value; }

  float get_fIangle() const { return header_.fIangle; }
  void set_fIangle(float value) { header_.fIangle=value; }

  float get_xorigin() const { return header_.fXoff; }
  void set_xorigin(float value) { header_.fXoff = value; }

  float get_yorigin() const { return header_.fYoff; }
  void set_yorigin(float value) { header_.fYoff = value; }

  float get_zorigin() const { return header_.fZoff; }
  void set_zorigin(float value) { header_.fZoff = value; }

  float get_object_pixel_size() const { return header_.fScale; }
  void set_object_pixel_size(float value) { header_.fScale = value; }

  float get_fLabbyt() const { return header_.fLabbyt; }
  void set_fLabbyt(float value) { header_.fLabbyt=value; }

  float get_fLenbyt() const { return header_.fLenbyt; }
  void set_fLenbyt(float value) { header_.fLenbyt=value; }

  double get_fGeo_matrix(unsigned int i,unsigned int j) const;

  //! Gets the fFlag.
  /** fFlag contains the number of triads of Euler angles stored
      in the header (up to three).
      set_euler_angles2 makes fFlag=2, set_euler_angles1 makes
      fFlag=max(fFlag, 1), set_euler_angles does not change fFlag
  */
  float get_fFlag() const { return header_.fFlag; }
  void set_fFlag(float value) { header_.fFlag = value; }


  // Euler angles
  template<typename T> void get_euler_angles(T& Phi, T& Theta, T& Psi) const {
    Phi =(T)header_.fPhi; Theta =(T)header_.fTheta; Psi =(T)header_.fPsi;
  }
  template<typename T> void get_euler_angles1(T& Phi, T& Theta, T& Psi) const {
   Phi=(T)header_.fPhi1; Theta=(T)header_.fTheta1; Psi=(T) header_.fPsi1;
  }
  template<typename T> void get_euler_angles2(T& Phi, T& Theta, T& Psi) const {
   Phi=(T)header_.fPhi2; Theta=(T)header_.fTheta2; Psi=(T) header_.fPsi2;
  }
  void set_euler_angles(float Phi, float Theta, float Psi) {
    header_.fIangle = 1;
    header_.fPhi = Phi; header_.fTheta = Theta; header_.fPsi = Psi;
  }
  void set_euler_angles1(float Phi, float Theta, float Psi) {
    if (header_.fFlag != 2.f) { header_.fFlag = 1.f; }
    header_.fPhi1 = Phi; header_.fTheta1 = Theta; header_.fPsi1 = Psi;
  }
  void set_euler_angles2(float Phi, float Theta, float Psi) {
    header_.fFlag = 2;
    header_.fPhi2 = Phi; header_.fTheta2 = Theta; header_.fPsi2 = Psi;
  }
  void set_phi(float value) { header_.fIangle = 1; header_.fPhi=value; }
  float get_phi() const { return header_.fPhi; }

  void set_theta(float value) { header_.fIangle = 1; header_.fTheta=value; }
  float get_theta() const { return header_.fTheta; }

  void set_psi(float value) { header_.fIangle = 1; header_.fPsi=value; }
  float get_psi() const { return header_.fPsi; }

  void set_phi1(float value) { header_.fFlag = 1.f; header_.fPhi1=value; }
  float get_phi1() const { return header_.fPhi1; }

  void set_theta1(float value) { header_.fFlag = 1.f; header_.fTheta1=value; }
  float get_theta1() const { return header_.fTheta1; }

  void set_psi1(float value) { header_.fFlag = 1.f; header_.fPsi1=value; }
  float get_psi1() const { return header_.fPsi1; }

  void set_phi2(float value) { header_.fFlag = 2.f; header_.fPhi2=value; }
  float get_phi2() const { return header_.fPhi2; }

  void set_theta2(float value) { header_.fFlag = 2.f; header_.fTheta2=value; }
  float get_theta2() const { return header_.fTheta2; }

  void set_psi2(float value) { header_.fFlag = 2.f; header_.fPsi2=value; }
  float get_psi2() const { return header_.fPsi2; }


  // Origin offsets
  template<typename T> void get_origin_offsets(T &Xoff, T &Yoff) const   {
    Yoff =  (T) header_.fYoff; Xoff =  (T) header_.fXoff;
  }
  template<typename T> void get_origin_offsets(T &Xoff,T &Yoff,T &Zoff) const {
    Zoff=(T)header_.fZoff; Yoff=(T)header_.fYoff; Xoff=(T)header_.fXoff;
  }
  void set_origin_offsets(float Xoff, float Yoff) {
    header_.fXoff = Xoff; header_.fYoff = Yoff;
  }
  void set_origin_offsets(float Xoff, float Yoff, float Zoff) {
    header_.fXoff = Xoff; header_.fYoff = Yoff; header_.fZoff = Zoff;
  }
  void set_origin_offsets(const algebra::Vector2D &v) {
    header_.fXoff=(float)v[0]; header_.fYoff=(float)v[1]; header_.fZoff=0.0;
  }
  void set_origin_offsets(const algebra::Vector3D &v) {
    header_.fXoff=(float)v[0];
    header_.fYoff=(float)v[1];
    header_.fZoff=(float)v[2];
  }

  bool is_normalized() const {
    if(std::abs(this->get_fAv()) <1e-6 &&
       std::abs(this->get_fSig()-1.0) < 1e-6) {
      return true;
    }
    return false;
  }

  // Date and Time
  char* get_date() const;
  char* get_time() const;
  void set_date();
  void set_time();



    //! Set and get info about image dimensions in the header
  void set_dimensions(float Ydim, float Xdim) {
    header_.fNrow = Ydim;
    header_.fNcol = Xdim;
  }

  void set_dimensions(float Zdim, float Ydim, float Xdim) {
    header_.fNslice = Zdim;
    header_.fNrow = Ydim;
    header_.fNcol = Xdim;
  }

    //! Get info about the image dimension from the header
  void get_dimensions(float &Ydim, float &Xdim) const {
    Ydim = header_.fNrow;
    Xdim = header_.fNcol;
  }

  void get_dimensions(float& Zdim,float& Ydim, float& Xdim) const {
    Zdim = header_.fNslice;
    Ydim = header_.fNrow;
    Xdim = header_.fNcol;
  }


  // Title
  char* get_title() const;

  //! Set title of image in the header
  void set_title(String newName);

private:

  // The header is directly in format Spider with a couple of additions
  em::SpiderHeader header_;
  bool reversed_;
}; // ImageHeader


IMP_OUTPUT_OPERATOR(ImageHeader);



IMPEM_END_NAMESPACE

#endif /* IMPEM_IMAGE_HEADER_H */
