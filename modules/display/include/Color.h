/**
 *  \file IMP/display/Color.h
 *  \brief Represent a color
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_COLOR_H
#define IMPDISPLAY_COLOR_H

#include <IMP/display/display_config.h>
#include <IMP/base/Value.h>
#include <IMP/base/showable_macros.h>
#include <IMP/base/comparison_macros.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/value_macros.h>
#include <boost/array.hpp>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent an RGB color
/**
 */
class IMPDISPLAYEXPORT Color: public base::Value
{
  boost::array<double, 3> c_;
  int compare(double a, double b) const {
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

  Color();

  /** Components must be between 0 and 1
   */
  Color(double r, double g, double b);

 /** Components must be between 0 and 1
   */
  template <class It>
  Color(It b, It e){
    std::copy(b,e, c_.begin());
  }

  ~Color();
  /** @name Component access
   @{
   */
  double get_red() const {
    return c_[0];
  }

  double get_green() const {
    return c_[1];
  }
  double get_blue() const {
    return c_[2];
  }
#ifndef SWIG
  typedef const double *ComponentIterator;
  ComponentIterator components_begin() const {
    return c_.begin();
  }
  ComponentIterator components_end() const {
    return c_.end();
  }
#endif
  //!@}

  void show(std::ostream &out, std::string delim) const {
    out << get_red() << delim << get_green() << delim << get_blue();
  }

  IMP_SHOWABLE_INLINE(Color, show(out, " "););

  IMP_COMPARISONS(Color);
};


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
  explicit SpacesIO(const Color &v): v_(v){}
};

struct CommasIO
{
  const Color &v_;
  explicit CommasIO(const Color &v): v_(v){}
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
    \relatesalso Color
 */
inline Color get_interpolated_rgb(const Color &a,
                                  const Color &b,
                                  double f) {
  return Color((1-f)*a.get_red()+f*b.get_red(),
               (1-f)*a.get_green()+f*b.get_green(),
               (1-f)*a.get_blue()+f*b.get_blue());
}

/** Return a number suitable for being passed to a color
    map that is gotten by linearly scaling between the
    passed min and max (and truncating values that are out
    of range).
*/
IMPDISPLAYEXPORT double get_linear_color_map_value(double min,
                                                   double max,
                                                   double value);

/** \name Colormaps
    These functions map a number in the interval [0,1]
    to a color using some color map.
@{
 */
//! Return the color for f from the jet color map
/** \relatesalso Color
 */
IMPDISPLAYEXPORT Color get_jet_color(double f);

//! Return the color for f from the hot color map
/** \relatesalso Color
 */
IMPDISPLAYEXPORT Color get_hot_color(double f);

//! Return the color for f from the rgb color map
/** \relatesalso Color
 */
IMPDISPLAYEXPORT Color get_rgb_color(double f);


//! Return the a greyscale value for f
/** \relatesalso Color
 */
IMPDISPLAYEXPORT Color get_grey_color(double f);

//! Return colors using the gnuplot default color map
/** \relatesalso Color
 */
IMPDISPLAYEXPORT Color get_gnuplot_color(double f);
/** @} */

IMP_VALUES(Color, Colors);

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_COLOR_H */
