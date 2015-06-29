/**
 *  \file KMrand.h
 *  \brief
 *
//----------------------------------------------------------------------
//      File:            KMrand.h
//      Programmer:      Sunil Arya and David Mount
//      Last modified:      03/27/02
//      Description:      Basic include file for random point generators
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
*/

#ifndef IMPKMEANS_INTERNAL_KMRAND_H
#define IMPKMEANS_INTERNAL_KMRAND_H

#include <IMP/kmeans/kmeans_config.h>

IMPKMEANS_BEGIN_INTERNAL_NAMESPACE

IMPKMEANSEXPORT int kmRanInt(  // random integer
    int n);                    // in the range [0,n-1]

IMPKMEANSEXPORT double kmRanUnif(  // random uniform in [lo,hi]
    double lo = 0.0, double hi = 1.0);

IMPKMEANS_END_INTERNAL_NAMESPACE

#endif /* IMPKMEANS_INTERNAL_KMRAND_H */
