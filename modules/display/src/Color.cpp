/**
 *  \file Color.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/Color.h"


IMPDISPLAY_BEGIN_NAMESPACE

Color::Color() {
  c_[0]=-1;
  c_[1]=-1;
  c_[2]=-1;
}

Color::Color( double r, double g, double b){
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

namespace {
  Color get_color_map_color(double f, Color *colors, unsigned int n) {
    IMP_USAGE_CHECK(f>=0 && f <= 1.0,
                    "Argument needs to be between 0 and 1.");
    int lb= static_cast<int>(std::floor(f*(n-1)));
    if (lb==static_cast<int>(n)) lb=n-2;
    if (lb<0) lb=0;
    double rem= (n-1)*f-lb;
    if (rem <0) rem=0;
    if (rem >1) rem=1;
    return get_interpolated_rgb(colors[lb], colors[lb+1], rem);
  }
}

Color get_jet_color(double f) {
  static Color colors[]={ Color(0,0,1),
                          Color(0,1,1),
                          Color(1,1,0),
                          Color(1,0,0),
                          Color(1,0,1),
                          Color(0,0,1)};
  return get_color_map_color(f, colors, sizeof(colors)/sizeof(Color));
}

Color get_rgb_color(double f) {
  static Color colors[]={ Color(0,0,1),
                          Color(0,1,0),
                          Color(1,0,0)};
  return get_color_map_color(f, colors, sizeof(colors)/sizeof(Color));
}


Color get_hot_color(double f) {
  static Color colors[]={Color(0,0,0),
                         Color(1,0,0),
                         Color(1,1,0),
                         Color(1,1,1)};
  return get_color_map_color(f, colors, sizeof(colors)/sizeof(Color));
}

Color get_grey_color(double f) {
  static Color colors[]={Color(0,0,0),
                         Color(1,1,1)};
  return get_color_map_color(f, colors, sizeof(colors)/sizeof(Color));
}

Color get_gnuplot_color(double f) {
  static Color colors[]={Color(0,0,0),
                         Color(.5,0,1),
                         Color(.6,0,.6),
                         Color(.75, .25, 0),
                         Color(.9, .5, 0),
                         Color(1,1,0)};
  return get_color_map_color(f, colors, sizeof(colors)/sizeof(Color));
}

double get_linear_color_map_value(double min,
                                  double max,
                                  double value) {
  IMP_USAGE_CHECK(min < max, "The min value is not less than the max value"
                  << min << " !< " << max);
  double sv= (value-min)/(max-min);
  if (sv < 0) return 0.0;
  else if (sv >1.0) return 1.0;
  else return sv;
}

IMPDISPLAY_END_NAMESPACE
