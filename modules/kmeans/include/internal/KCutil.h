/**
 *  \file KCutil.h
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
//----------------------------------------------------------------------
//      File:            KCutil.h
//      Programmer:      David Mount
//      Last modified:      03/27/02
//      Description:      Declarations for kc-tree utilities
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

#ifndef IMPKMEANS_INTERNAL_KCUTIL_H
#define IMPKMEANS_INTERNAL_KCUTIL_H

#include <IMP/kmeans/kmeans_config.h>
#include "KCtree.h"  // kc-tree declarations

IMPKMEANS_BEGIN_INTERNAL_NAMESPACE

//----------------------------------------------------------------------
//  externally accessible functions
//----------------------------------------------------------------------

// compute smallest enclosing rectangle
IMPKMEANSEXPORT void kmEnclRect(KMpointArray pa,    // point array
                                KMidxArray pidx,    // point indices
                                int n,              // number of points
                                int dim,            // dimension
                                KMorthRect &bnds);  // bounding cube (returned)

// compute point spread along dimension
IMPKMEANSEXPORT KMcoord kmSpread(KMpointArray pa,  // point array
                                 KMidxArray pidx,  // point indices
                                 int n,            // number of points
                                 int d);           // dimension to check

// compute min and max coordinates along dim
IMPKMEANSEXPORT void kmMinMax(KMpointArray pa,  // point array
                              KMidxArray pidx,  // point indices
                              int n,            // number of points
                              int d,            // dimension to check
                              KMcoord &min,     // minimum value (returned)
                              KMcoord &max);    // maximum value (returned)

// split points by a plane
IMPKMEANSEXPORT void kmPlaneSplit(KMpointArray pa,  // points to split
                                  KMidxArray pidx,  // point indices
                                  int n,            // number of points
                                  int d,       // dimension along which to split
                                  KMcoord cv,  // cutting value
                                  int &br1,    // first break (values < cv)
                                  int &br2);   // second break (values == cv)

// sliding midpoint kd-splitter
IMPKMEANSEXPORT void sl_midpt_split(
    KMpointArray pa,         // point array (unaltered)
    KMidxArray pidx,         // point indices (permuted on return)
    const KMorthRect &bnds,  // bounding rectangle for cell
    int n,                   // number of points
    int dim,                 // dimension of space
    int &cut_,               // cutting dimension (returned)
    KMcoord &cut_val,        // cutting value (returned)
    int &                    // num of points on low side (returned)
    );

IMPKMEANS_END_INTERNAL_NAMESPACE

#endif /* IMPKMEANS_INTERNAL_KCUTIL_H */
