/**
 *  \file Pixel.h
 *  \brief A class to deal with pixels comfortably
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPEM2D_PIXEL_H
#define IMPEM2D_PIXEL_H


#include "IMP/em2d/em2d_config.h"
#include <IMP/macros.h>
#include <IMP/exception.h>
#include <vector>

IMPEM2D_BEGIN_NAMESPACE

//! A class to deal with 2D pixels comfortably
class Pixel : public std::vector<int>
{
public:
  Pixel() {
    this->resize(2);
    this->at(0)=0;this->at(1)=0;
    null_ = false;
  }

  Pixel(int i,int j) {
    this->resize(2);
    this->at(0)=i;this->at(1)=j;
    null_ = false;
  }

  int operator[](unsigned int i) const {
    IMP_INTERNAL_CHECK((i==0 || i==1), "Invalid component of Pixel requested.");
    IMP_INTERNAL_CHECK(this->get_is_null() == false,
                            "Error: Accessing a null pixel.");
    return this->at(i);
  }

  int& operator[](unsigned int i) {
    IMP_INTERNAL_CHECK((i==0 || i==1), "Invalid component of Pixel requested.");
    IMP_INTERNAL_CHECK(this->get_is_null() == false,
                            "Error: Accessing a null pixel.");
    return this->at(i);
  }

  Pixel operator-(const Pixel &p) const {
    Pixel q(this->at(0)-p[0],this->at(1)-p[1]);
    return q;
  }

  Pixel operator+(const Pixel &p) const {
    Pixel q(this->at(0)+p[0],this->at(1)+p[1]);
    return q;
  }

  void operator=(const Pixel &v) {
    this->resize(2);
    this->at(0)=v[0];
    this->at(1)=v[1];
  }


  void show(std::ostream& os) const {
    os << "Pixel: " << this->at(0) << "," << this->at(1);
  }

  void set_null(bool q) {
    null_=q;
  }

  bool get_is_null() const {
    return null_;
  }
private:
  bool null_;
};

IMP_OUTPUT_OPERATOR(Pixel);

IMP_VALUES(Pixel,Pixels);

IMPEM2D_END_NAMESPACE

#endif /* IMPEM2D_PIXEL_H */
