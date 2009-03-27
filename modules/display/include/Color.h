/**
 *  \file Color.h
 *  \brief Represent a color
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_COLOR_H
#define IMPDISPLAY_COLOR_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a color
/**
 */
class IMPDISPLAYEXPORT Color
{
  float c_[3];
  bool is_default() const {
    return c_[0] < 0;
  }
public:
  typedef Color This;

  Color();

  //! set the color from r,g,b components
  /** Components must be between 0 and 1
   */
  Color(float r, float g, float b);

  ~Color();

  //! Red rgb component, from 0 to 1
  float get_red() const {
    return c_[0];
  }

  //! Green rgb component, from 0 to 1
  float get_green() const {
    return c_[1];
  }

  //! Blye rgb component, from 0 to 1
  float get_blue() const {
    return c_[2];
  }

  void show(std::ostream &out=std::cout, std::string delim=" ") const {
    out << get_red() << delim << get_green() << delim << get_blue();
  }

  IMP_COMPARISONS_3(c_[0],c_[1], c_[2]);
};

IMP_OUTPUT_OPERATOR(Color)


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
#endif

//! Use this before outputing to delimited vector entries with a space
/** std::cout << spaces_io(v);
    produces "1.0 2.0 3.0"
    \relates Color
 */
inline SpacesIO spaces_io(const Color &v) {
  return SpacesIO(v);
}




//! Use this before outputing to delimited vector entries with a comma
/** std::cout << commas_io(v);
    produces "1.0, 2.0, 3.0"
    \relates Color
 */
inline CommasIO commas_io(const Color &v) {
  return CommasIO(v);
}

#endif // SWIG

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_COLOR_H */
