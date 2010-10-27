/**
 *  \file Color.h
 *  \brief Represent a color
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_COLOR_H
#define IMPDISPLAY_COLOR_H

#include "display_config.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent an RGB color
/**
 */
class IMPDISPLAYEXPORT Color
{
  float c_[3];
  int compare(float a, float b) const {
    if (a< b) return -1;
    else if (a > b) return 1;
    else return 0;
  }
  int compare(const Color &o) const {
    for (unsigned int i=0; i< 3; ++i) {
      int c= compare(c_[i], o.c_[i]);
      if (c != 0) return c;
    }
    return 0;
  }
public:
  typedef Color This;

  Color();

  /** Components must be between 0 and 1
   */
  Color(float r, float g, float b);

  ~Color();
  /** @name Component access
   @{
   */
  float get_red() const {
    return c_[0];
  }

  float get_green() const {
    return c_[1];
  }
  float get_blue() const {
    return c_[2];
  }
  //!@}

  void show(std::ostream &out=std::cout, std::string delim=" ") const {
    out << get_red() << delim << get_green() << delim << get_blue();
  }

  IMP_COMPARISONS;
};

IMP_OUTPUT_OPERATOR(Color);


/** Produce a color that is attempted to be contrasting with the
    i-1 previous colors. Currently, they recycle after 11, but they
    can be darkened instead. Just ask.
 */
IMPDISPLAYEXPORT Color get_display_color(unsigned int i);

#ifndef SWIG
#ifndef IMP_DOXYGEN
struct SpacesIO
{
  const Color &v_;
  SpacesIO(const Color &v): v_(v){}
};

struct CommasIO
{
  const Color &v_;
  CommasIO(const Color &v): v_(v){}
};
inline std::ostream &operator<<(std::ostream &out, const SpacesIO &s)
{
  s.v_.show(out, " ");
  return out;
}
inline std::ostream &operator<<(std::ostream &out, const CommasIO &s)
{
  s.v_.show(out, ", ");
  return out;
}

//! Use this before outputing to delimited vector entries with a space
/** std::cout << spaces_io(v);
    produces "1.0 2.0 3.0"
    \relatesalso Color
 */
inline SpacesIO spaces_io(const Color &v) {
  return SpacesIO(v);
}




//! Use this before outputing to delimited vector entries with a comma
/** std::cout << commas_io(v);
    produces "1.0, 2.0, 3.0"
    \relatesalso Color
 */
inline CommasIO commas_io(const Color &v) {
  return CommasIO(v);
}

//! Multiply a color by a value less than 1
/** \unstable{Color multiplication}
 */
inline Color operator*(double d, Color c) {
  IMP_USAGE_CHECK(d <=1 && d>=0, "Colors can only fade with multiplication");
  return Color(c.get_red()*d, c.get_green()*d, c.get_blue()*d);
}

//! Multiply a color by a value less than 1
/** \unstable{Color multiplication}
 */
inline Color operator*(Color c, double d) {
  return d*c;
}
#endif

#endif // SWIG

//! Return a color interpolated between a and b in RGB space
/** If f is 0, then a is returned, if f is 1, b is returned.
 */
inline Color get_interpolated_rgb(const Color &a,
                                  const Color &b,
                                  double f) {
  return Color((1-f)*a.get_red()+f*b.get_red(),
               (1-f)*a.get_green()+f*b.get_green(),
               (1-f)*a.get_blue()+f*b.get_blue());
}


//! Return the color for f from the jet color map
/** The argument must be between 0 and 1, inclusive.
 */
IMPDISPLAYEXPORT Color get_jet_color(double f);

IMP_VALUES(Color, Colors);

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_COLOR_H */
