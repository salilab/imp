/**
 *  \file Color.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
  IMP_USAGE_CHECK(r>= 0 && r <=1, "Red out of range: " << r);
  IMP_USAGE_CHECK(g>= 0 && g <=1, "Green out of range: " << g);
  IMP_USAGE_CHECK(b>= 0 && b <=1, "Blue out of range: " << b);
  c_[0]=r;
  c_[1]=g;
  c_[2]=b;
}

Color::~Color(){}

Color get_display_color(unsigned int i) {
  // from http://colorbrewer2.org/
  static const Color all[]={
    Color(166/255.,206./255.,227./255.),
    Color(31./255., 120./255., 180./255.),
    Color(178./255., 223./255., 138./255.),
    Color(51./255., 160./255., 44./255.),
    Color(251./255., 154./255., 153./255.),
    Color(227./255., 26./255., 28./255.),
    Color(253./255., 191./255., 111./255.),
    Color(255./255., 127./255., 0./255.),
    Color(202./255., 178./255., 214./255.),
    Color(106./255., 61./255., 154./255.),
    Color(255./255., 255./255., 153./255.)
  };
  static const int size=sizeof(all)/sizeof(Color);
  return all[i%size];
}


Color get_jet_color(double f) {
  IMP_USAGE_CHECK(f>=0 && f <= 1.0,
                  "Argument needs to be between 0 and 1.");
  static Color colors[]={ Color(0,0,1),
                        Color(0,1,1),
                        Color(1,1,0),
                        Color(1,0,0),
                        Color(1,0,1),
                        Color(0,0,1)};
  int lb= static_cast<int>(std::floor(f*6));
  if (lb==6) lb=5;
  double rem= 6*f-lb;
  if (rem <0) rem=0;
  if (rem >1) rem=1;
  return get_interpolated_rgb(colors[lb], colors[lb+1], rem);
}


IMPDISPLAY_END_NAMESPACE
