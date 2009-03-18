/**
 *  \file output_helpers.cpp
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *  Adapted with permission from Xmip 2.2
*/
#include "IMP/algebra/internal/output_helpers.h"
#include <cstdio>
#include <cmath>
#include <cerrno>


#include <sstream>


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

int best_precision(double F, Int width)
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

std::string float_to_string(double F, Int width, Int pre)
{
  std::ostringstream outs;

  outs.fill(' ');
  if (width != 0) {
    outs.width(width);
  }
  if (pre == 0) {
    pre = best_precision(F, width);
  }
  if (pre == -1 && width > 7) {
    outs.precision(width - 7);
    outs.setf(std::ios::scientific);
  }
  else {
    outs.precision(pre);
  }
  outs << F;

  String retval = outs.str();
  Int i = retval.find('\0');

  if (i != -1)
    retval = retval.substr(0, i);

  return retval;
}

IMPALGEBRA_END_INTERNAL_NAMESPACE
