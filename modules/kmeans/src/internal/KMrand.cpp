/**
 *  \file KMrand.cpp
 *  \brief
*/

//----------------------------------------------------------------------
//      File:            KMrand.cpp
//      Programmer:      Sunil Arya and David Mount
//      Last modified:      05/14/04
//      Description:      Routines for random point generation
//----------------------------------------------------------------------
// Copyright (C) 2004-2005 David M. Mount and University of Maryland
// All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at
// your option) any later version.  See the file Copyright.txt in the
// main directory.
//
// The University of Maryland and the authors make no representations
// about the suitability or fitness of this software for any purpose.
// It is provided "as is" without express or implied warranty.
//----------------------------------------------------------------------

#include "IMP/kmeans/internal/KMrand.h"  // random generator declarations
#include <IMP/random.h>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>

IMPKMEANS_BEGIN_INTERNAL_NAMESPACE

//------------------------------------------------------------------------
//  kmRanInt - generate a random integer from {0,1,...,n-1}
//
//      If n == 0, then -1 is returned.
//------------------------------------------------------------------------

int kmRanInt(int n) {
  /* Modified for IMP: use IMP's random number generator instead */
  if (n == 0) {
    return -1;
  } else {
    ::boost::uniform_int<int> randint(0, n - 1);
    return randint(random_number_generator);
  }
}

//------------------------------------------------------------------------
//  kmRanUnif - generate a random uniform in [lo,hi]
//------------------------------------------------------------------------
double kmRanUnif(double lo, double hi) {
  /* Modified for IMP: use IMP's random number generator instead */
  ::boost::uniform_real<double> randfloat(lo, hi);
  return randfloat(random_number_generator);
}

IMPKMEANS_END_INTERNAL_NAMESPACE
