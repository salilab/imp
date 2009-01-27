/**
 *  \file text_manipulation.cpp
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *  Adapted with permission from Xmip 2.2
*/

#include "IMP/base_types.h"
#include "IMP/misc/text_manipulation.h"
#include <cstdio>
#include <cmath>
#include <cerrno>

IMPMISC_BEGIN_NAMESPACE

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.3.0 */
#if GCC_VERSION >= 30300
#include <sstream>
#else
#include <strstream>
#endif

Int best_precision(Float F, Int width)
{
  // Trivial case
  if (F == 0) {
    return 1;
  }
  // Otherwise
  Int exp = std::floor(log10(std::abs(F)));
  Int advised_prec;

  if (exp >= 0)
    if (exp > width - 3)
      advised_prec = -1;
    else
      advised_prec = width - 2;
  else {
    advised_prec = width + (exp - 1) - 3;
    if (advised_prec <= 0)
      advised_prec = -1;
  }

  if (advised_prec < 0)
    advised_prec = -1; // Choose exponential format

  return advised_prec;
}

String float_to_string(Float F, Int width, Int _prec)
{
#if GCC_VERSION < 30300
  char aux[15];
  std::ostrstream outs(aux, sizeof(aux));
#else
  std::ostringstream outs;
#endif

  outs.fill(' ');
  if (width != 0) {
    outs.width(width);
  }
  if (_prec == 0) {
    _prec = best_precision(F, width);
  }
  if (_prec == -1 && width > 7) {
    outs.precision(width - 7);
    outs.setf(std::ios::scientific);
  }
  else {
    outs.precision(_prec);
  }
#if GCC_VERSION < 30301
  outs << F << std::ends;
#else
  outs << F;
#endif
#if GCC_VERSION < 30300
  return String(aux);
#else
  String retval = outs.str();
  Int i = retval.find('\0');

  if (i != -1)
    retval = retval.substr(0, i);

  return retval;
#endif
}

IMPMISC_END_NAMESPACE
