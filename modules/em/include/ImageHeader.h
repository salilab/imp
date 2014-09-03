/**
 *  \file IMP/em/ImageHeader.h
 *  \brief Header for EM images. Compatible with Spider and Xmipp formats
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
**/

#ifndef IMPEM_IMAGE_HEADER_H
#define IMPEM_IMAGE_HEADER_H

#include "IMP/em/em_config.h"
#include "IMP/em/SpiderHeader.h"
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Vector2D.h>
#include <IMP/algebra/utility.h>
#include <IMP/algebra/endian.h>
#include <IMP/base/showable_macros.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>

IMPEM_BEGIN_NAMESPACE

//! Class to deal with the header of Electron Microscopy images in IMP.
/*!
  \note Compatible with SPIDER format
*/
class IMPEMEXPORT ImageHeader {
 public:
  //! Types of initialization of ImageHeader
  typedef enum {
    IMG_BYTE = 0,
    IMG_IMPEM = 1,
    IMG_INT = 9, /*IMG_SHORT  = , IMG_UCHAR = ,*/
    VOL_BYTE = 2,
    VOL_IMPEM = 3,
    VOL_INT = 10, /*VOL_SHORT  = , VOL_UCHAR =, */
    IMG_FOURIER = -1,
    VOL_FOURIER = -3
  } img_type;

  ImageHeader() {
    clear();
    set_image_type((float)IMG_IMPEM);
  }

  ImageHeader(img_type im) {
    clear();
    set_image_type((float)im);
  }

  ImageHeader(float im) {
    clear();
    set_image_type(im);
  }

  float get_image_type() const { return spider_header_.fIform; }

  //! Set the type of image
  /**
    \param[in] im type of image. Available values:
   IMG_BYTE = 0, IMG_IMPEM  = 1,IMG_INT  = 9, VOL_BYTE = 2,  VOL_IMPEM  = 3,
   VOL_INT  = 10 , IMG_FOURIER = -1, VOL_FOURIER = -3
   \note No check is done for correctness of the value
  */
  void set_image_type(img_type im) { set_image_type((float)im); }

  void set_image_type(float im) { spider_header_.fIform = im; }

  void do_show(std::ostream& out) const {
    out << "Image type   : ";
    switch ((int)spider_header_.fIform) {
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
    if (reversed_) {
      out << "TRUE" << std::endl;
    } else {
      out << "FALSE" << std::endl;
    }
    out << "dimensions  (slices x rows x columns) : " << get_number_of_slices()
        << " " << get_number_of_rows() << " " << get_number_of_columns()
        << std::endl;
    out << "Origin (shift) : " << get_origin() << std::endl;
    out << "Euler angles (Phi, Theta, Psi) (ZYZ convention): "
        << get_euler_angles() << std::endl;
    if (spider_header_.fFlag == 1.0f || spider_header_.fFlag == 2.0f) {
      out << "Euler angles (Phi1, Theta1, Psi1) (ZYZ convention): "
          << get_euler_angles1() << std::endl;
    }
    if (spider_header_.fFlag == 2.0f) {
      out << "Euler angles (Phi2, Theta2, Psi2) (ZYZ convention): "
          << get_euler_angles1() << std::endl;
    }

    out << "Date         : " << get_date() << std::endl;
    out << "Time         : " << get_time() << std::endl;
    out << "Title        : " << get_title() << std::endl;
    out << "Header size  : " << get_spider_header_size() << std::endl;
    out << "Weight  : " << get_Weight() << std::endl;
  }

  IMP_SHOWABLE_INLINE(ImageHeader, do_show(out));
  //! Prints a reduced set of information (debugging purposes)
  void print_hard(std::ostream& out) const;

  //! Shows only the projection parameters
  inline void show_projection_params(std::ostream& out) const {
    out << "(Phi,Theta,Psi) = ( " << spider_header_.fPhi << " , "
        << spider_header_.fTheta << " , " << spider_header_.fPsi << " ) "
        << " (x,y) = ( " << spider_header_.fXoff << " , "
        << spider_header_.fYoff << " ) " << std::endl;
  }

  //! Reads the header of a EM image
  // \note reversed is only used in case that the type_check is skipped
  int read(const String filename, bool skip_type_check = false,
           bool force_reversed = false, bool skip_extra_checkings = false);

  //! Reads the header of a EM image from an input file stream
  bool read(std::ifstream& f, bool skip_type_check = false,
            bool force_reversed = false, bool skip_extra_checkings = false);

  //! Writes the header of a EM image
  void write(const String& filename, bool force_reversed = false);

  //! Writes the header of a EM image to an output file stream
  void write(std::ofstream& f, bool force_reversed = false);

  //! Clear header data and sets a consistent header
  void clear();

  //! Sets a consistent header
  void set_header();

  //! Interaction with data
  bool get_reversed() const { return reversed_; }
  void set_reversed(bool value) { reversed_ = value; }

  unsigned int get_number_of_slices() const {
    return (unsigned int)spider_header_.fNslice;
  }
  void set_number_of_slices(unsigned int n) {
    spider_header_.fNslice = (float)n;
  }

  unsigned int get_number_of_rows() const {
    return (unsigned int)spider_header_.fNrow;
  }
  void set_number_of_rows(unsigned int n) { spider_header_.fNrow = (float)n; }

  unsigned int get_number_of_columns() const {
    return (unsigned int)spider_header_.fNcol;
  }
  void set_number_of_columns(unsigned int n) {
    spider_header_.fNcol = (float)n;
  }

  //! get rotation angle. (Xmipp compatibility)
  float get_old_rot() const { return spider_header_.fAngle1; }

  //! set rotation angle. (Xmipp compatibility)
  void set_old_rot(float value) { spider_header_.fAngle1 = value; }

  //! get rotation angle. (Xmipp compatibility)
  float get_fAngle1() const { return spider_header_.fAngle1; }

  //! set rotation angle. (Xmipp compatibility)
  void set_fAngle1(float value) { spider_header_.fAngle1 = value; }

  float get_Scale() const { return spider_header_.fScale; }

  void set_Scale(float value) { spider_header_.fScale = value; }

  /* For Maximum-Likelihood refinement (Xmipp compatibility:
  not currently used)
  */
  float get_Flip() const { return spider_header_.Flip; }

  /* For Maximum-Likelihood refinement (Xmipp compatibility:
  not currently used)
  */
  void set_Flip(float value) { spider_header_.Flip = value; }

  float get_Weight() const { return spider_header_.Weight; }
  void set_Weight(float value) { spider_header_.Weight = value; }

  float get_fNrec() const { return spider_header_.fNrec; }
  void set_fNrec(float value) { spider_header_.fNrec = value; }

  float get_fNlabel() const { return spider_header_.fNlabel; }
  void set_fNlabel(float value) { spider_header_.fNlabel = value; }

  float get_fIform() const { return spider_header_.fIform; }
  void set_fIform(float value) { spider_header_.fIform = value; }

  float get_fImami() const { return spider_header_.fImami; }
  void set_fImami(float value) { spider_header_.fImami = value; }

  float get_fFmax() const { return spider_header_.fFmax; }
  void set_fFmax(float value) { spider_header_.fFmax = value; }

  float get_fFmin() const { return spider_header_.fFmin; }
  void set_fFmin(float value) { spider_header_.fFmin = value; }

  float get_fAv() const { return spider_header_.fAv; }
  void set_fAv(float value) { spider_header_.fAv = value; }

  float get_fSig() const { return spider_header_.fSig; }
  void set_fSig(float value) { spider_header_.fSig = value; }

  float get_fIhist() const { return spider_header_.fIhist; }
  void set_fIhist(float value) { spider_header_.fIhist = value; }

  float get_fLabrec() const { return spider_header_.fLabrec; }
  void set_fLabrec(float value) { spider_header_.fLabrec = value; }

  float get_fIangle() const { return spider_header_.fIangle; }
  void set_fIangle(float value) { spider_header_.fIangle = value; }

  algebra::Vector3D get_origin() const {
    return algebra::Vector3D(spider_header_.fXoff, spider_header_.fYoff,
                             spider_header_.fZoff);
  };

  void set_origin(const algebra::Vector3D& v) {
    spider_header_.fXoff = v[0];
    spider_header_.fYoff = v[1];
    spider_header_.fZoff = v[2];
  }

  void set_origin(const algebra::Vector2D& v) {
    spider_header_.fXoff = (float)v[0];
    spider_header_.fYoff = (float)v[1];
    spider_header_.fZoff = (float)0.0;
  }

  float get_object_pixel_size() const { return spider_header_.fScale; }
  void set_object_pixel_size(float value) { spider_header_.fScale = value; }

  float get_fLabbyt() const { return spider_header_.fLabbyt; }
  void set_fLabbyt(float value) { spider_header_.fLabbyt = value; }

  float get_fLenbyt() const { return spider_header_.fLenbyt; }
  void set_fLenbyt(float value) { spider_header_.fLenbyt = value; }

  double get_fGeo_matrix(unsigned int i, unsigned int j) const;

  //! Gets the fFlag.
  /** fFlag contains the number of triads of Euler angles stored
      in the header (up to three).
      set_euler_angles2 makes fFlag=2, set_euler_angles1 makes
      fFlag=max(fFlag, 1), set_euler_angles does not change fFlag
  */
  float get_fFlag() const { return spider_header_.fFlag; }
  void set_fFlag(float value) { spider_header_.fFlag = value; }

  algebra::Vector3D get_euler_angles() const {
    return algebra::Vector3D(spider_header_.fPhi, spider_header_.fTheta,
                             spider_header_.fPsi);
  }

  algebra::Vector3D get_euler_angles1() const {
    return algebra::Vector3D(spider_header_.fPhi1, spider_header_.fTheta1,
                             spider_header_.fPsi1);
  }

  algebra::Vector3D get_euler_angles2() const {
    return algebra::Vector3D(spider_header_.fPhi2, spider_header_.fTheta2,
                             spider_header_.fPsi2);
  }

  void set_euler_angles(const algebra::Vector3D& euler) {
    spider_header_.fIangle = 1;
    spider_header_.fPhi = euler[0];    // z
    spider_header_.fTheta = euler[1];  // y
    spider_header_.fPsi = euler[2];    // z
  }

  void set_euler_angles1(const algebra::Vector3D& euler) {
    if (spider_header_.fFlag != 2.f) {
      spider_header_.fFlag = 1.f;
    }
    spider_header_.fPhi1 = euler[0];    // z
    spider_header_.fTheta1 = euler[1];  // y
    spider_header_.fPsi1 = euler[2];    // z
  }

  void set_euler_angles2(const algebra::Vector3D& euler) {
    spider_header_.fFlag = 2;
    spider_header_.fPhi2 = euler[0];    // z
    spider_header_.fTheta2 = euler[1];  // y
    spider_header_.fPsi2 = euler[2];    // z
  }

  bool is_normalized() const {
    if (std::abs(get_fAv()) < 1e-6 && std::abs(get_fSig() - 1.0) < 1e-6) {
      return true;
    }
    return false;
  }

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  // Date and Time
  char* get_date() const;
  char* get_time() const;
#endif
  void set_date();
  void set_time();

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  // Title
  char* get_title() const;
#endif

  //! Set title of image in the header
  void set_title(String newName);

 private:
  //! get header size
  int get_spider_header_size() const {
    return (int)spider_header_.fNcol * (int)spider_header_.fLabrec *
           sizeof(float);
  }

  // The header is directly in format Spider with a couple of additions
  em::SpiderHeader spider_header_;
  bool reversed_;
};  // ImageHeader

IMPEM_END_NAMESPACE

#endif /* IMPEM_IMAGE_HEADER_H */
