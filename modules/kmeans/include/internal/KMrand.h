/**
 *  \file KMrand.h
 *  \brief
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
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

//----------------------------------------------------------------------
//  Basic includes
//----------------------------------------------------------------------
#include <IMP/kmeans/kmeans_config.h>
#include <cstdlib>   // standard C++ includes
#include <math.h>    // math routines
#include "KMeans.h"  // KMeans includes

IMPKMEANS_BEGIN_INTERNAL_NAMESPACE

//----------------------------------------------------------------------
//  Globals
//----------------------------------------------------------------------
IMPKMEANSEXPORT extern int kmIdum;  // used for random number generation

//----------------------------------------------------------------------
//  External entry points
//----------------------------------------------------------------------

IMPKMEANSEXPORT int kmRanInt(  // random integer
    int n);                    // in the range [0,n-1]

IMPKMEANSEXPORT double kmRanUnif(  // random uniform in [lo,hi]
    double lo = 0.0, double hi = 1.0);

IMPKMEANSEXPORT void kmUniformPts(  // uniform distribution
    KMpointArray pa,                // point array (modified)
    int n,                          // number of points
    int dim);                       // dimension

IMPKMEANSEXPORT void kmGaussPts(  // Gaussian distribution
    KMpointArray pa,              // point array (modified)
    int n,                        // number of points
    int dim,                      // dimension
    double std_dev);              // standard deviation

IMPKMEANSEXPORT void kmCoGaussPts(  // correlated-Gaussian distribution
    KMpointArray pa,                // point array (modified)
    int n,                          // number of points
    int dim,                        // dimension
    double correlation);            // correlation

IMPKMEANSEXPORT void kmLaplacePts(  // Laplacian distribution
    KMpointArray pa,                // point array (modified)
    int n,                          // number of points
    int dim);                       // dimension

IMPKMEANSEXPORT void kmCoLaplacePts(  // correlated-Laplacian distribution
    KMpointArray pa,                  // point array (modified)
    int n,                            // number of points
    int dim,                          // dimension
    double correlation);              // correlation

IMPKMEANSEXPORT void kmClusGaussPts(  // clustered-Gaussian distribution
    KMpointArray pa,                  // point array (modified)
    int n,                            // number of points
    int dim,                          // dimension
    int n_col,                        // number of colors (clusters)
    bool new_clust = true,            // generate new cluster centers
    double std_dev = 0.1,             // std deviation within clusters
    double* clus_sep = NULL);         // cluster separation (returned)

// get clustered-gauss cluster centers
IMPKMEANSEXPORT KMpointArray kmGetCGclusters();

// clustered along orthogonal flats
IMPKMEANSEXPORT void kmClusOrthFlats(
    KMpointArray pa,  // point array (modified)
    int n,            // number of points
    int dim,          // dimension
    int n_col,        // number of colors
    bool new_clust,   // generate new clusters.
    double std_dev,   // standard deviation within clusters
    int max_dim);     // maximum dimension of the flats

// clustered around ellipsoids
IMPKMEANSEXPORT void kmClusEllipsoids(
    KMpointArray pa,       // point array (modified)
    int n,                 // number of points
    int dim,               // dimension
    int n_col,             // number of colors
    bool new_clust,        // generate new clusters.
    double std_dev_small,  // small standard deviation
    double std_dev_lo,     // low standard deviation for ellipses
    double std_dev_hi,     // high standard deviation for ellipses
    int max_dim);          // maximum dimension of the flats

// multi-sized clusters
IMPKMEANSEXPORT void kmMultiClus(KMpointArray pa,  // point array (modified)
                                 int n,            // number of points
                                 int dim,          // dimension
                                 int& k,  // number of clusters (returned)
                                 double base_dev);  // base standard deviation

IMPKMEANS_END_INTERNAL_NAMESPACE

#endif /* IMPKMEANS_INTERNAL_KMRAND_H */
