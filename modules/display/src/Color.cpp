/**
 *  \file Color.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/Color.h"


IMPDISPLAY_BEGIN_NAMESPACE

Color::Color() {
  c_[0]=-1;
  c_[1]=-1;
  c_[2]=-1;
}

Color::Color( float r, float g, float b){
  IMP_check(r>= 0 && r <=1, "Red out of range: " << r,
            ValueException);
  IMP_check(g>= 0 && g <=1, "Green out of range: " << g,
            ValueException);
  IMP_check(b>= 0 && b <=1, "Blue out of range: " << b,
            ValueException);
  c_[0]=r;
  c_[1]=g;
  c_[2]=b;
}

Color::~Color(){}

IMPDISPLAY_END_NAMESPACE
