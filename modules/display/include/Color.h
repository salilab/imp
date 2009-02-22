/**
 *  \file Color.h
 *  \brief Represent a color
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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

  void show(std::ostream &out=std::cout) const {
    out << get_red() << " " << get_green() << " " << get_blue();
  }

  IMP_COMPARISONS_3(c_[0],c_[1], c_[2]);
};

IMP_OUTPUT_OPERATOR(Color)

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_COLOR_H */
