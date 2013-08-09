/**
 *  \file KMeans.h
 *  \brief
 *
 *  Copyright 2007-2013 IMP Inventors. All rights
//----------------------------------------------------------------------
//      File:           KMeans.h
//      Programmer:     David Mount
//      Last modified:  08/10/05
//      Description:    Include file for kmeans algorithms.
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
// History: (Changes to source code)
//  Version 1.0     04/29/2002
//      Initial release.
//  Version: 1.01   10/02/2002
//      Modified output levels.
//  Version: 1.1    04/08/2003
//      Added EZ_Hybrid and dampening.  Fixed memory leaks.
//  Version: 1.2    09/13/2003
//      Added sample programs, kmlsample.cpp and kmlminimal.cpp.
//  Version: 1.5    05/14/2004
//      Changed sample program kmlsample to allow random point
//      generation.  Made minor changes for compilation under Redhat
//      Linux and Visual Studio.NET.
//  Version: 1.6    03/09/2005
//      Fixed memory leak in KMfilterCenters.cpp.  Fixed random
//      number error for Microsoft Visual C++.
//  Version: 1.7    08/10/2005
//      Added capability for reporting final assignment to clusters.
//----------------------------------------------------------------------
*/

#ifndef IMPKMEANS_INTERNAL_KMEANS_H
#define IMPKMEANS_INTERNAL_KMEANS_H

#include <IMP/kmeans/kmeans_config.h>
#include <IMP/base/log.h>
#include "KM_ANN.h"                  // basic definitions

IMPKMEANS_BEGIN_INTERNAL_NAMESPACE


//----------------------------------------------------------------------
//  Important strings
//----------------------------------------------------------------------
const std::string KMshortName    = "KMlocal";
const std::string KMlongName          =
  "KMlocal (k-means clustering by local search)";
const std::string KMversion          = "1.7";
const std::string KMversionCmt   = "(Use at your own risk)";
const std::string KMcopyright    = "David M. Mount";
const std::string KMlatestRev    = "August 10, 2005";

//------------------------------------------------------------------------
//  Type definitions
//      Although data points and centers are of the same type
//      as a KMpoint, we distinguish these types here for the
//      sake of making the code a little easier to interpret.
//
//      KMdataPoint      Used for k-means data points.
//      KMcenter      Used for k-means center points.
//      KMpoint            Used for any other points and intermediate
//                  results used in the program.
//------------------------------------------------------------------------

typedef KMpoint            KMdataPoint;      // data point
typedef KMpoint            KMcenter;      // center point

typedef KMpointArray      KMdataArray;      // array of data points
typedef KMpointArray      KMcenterArray;      // array of center

typedef KMidx            KMdataIdx;      // a data point index
typedef KMidx            KMctrIdx;      // a center point index
typedef KMdataIdx      *KMdatIdxArray;      // array of data indices
typedef KMctrIdx      *KMctrIdxArray;      // array of center indices

//------------------------------------------------------------------------
//  Global constants
//------------------------------------------------------------------------

const double KM_ERR       = 1E-6;      // epsilon (for floating compares)
const double KM_HUGE       = DBL_MAX;      // huge double value
const int    KM_HUGE_INT = INT_MAX;      // huge int value

enum KMerr {KMwarn = 0, KMabort = 1};      // what to do in case of error

enum StatLev {                        // output statistics levels
  SILENT,                        // no output
  EXEC_TIME,                  // just execution time
  SUMMARY,                  // summary of entire algorithm
  PHASE,                        // summary of each phase
  RUN,                        // summary of each run
  STAGE,                        // summary of each stage
  STEP,                        // summary of each step
  CENTERS,                  // output centers with each step
  TREE,                        // output tree and points
  N_STAT_LEVELS
};                  // number of levels

enum KMalg {                        // k-means algorithm names
  LLOYD,                        // Lloyd's (using filtering)
  SWAP,                        // swap heuristic
  HYBRID,                        // hybrid algorithm
  EZ_HYBRID,                  // EZ-hybrid algorithm
  RANDOM,                        // random centers
  N_KM_ALGS
};                  // number of algorithms

//----------------------------------------------------------------------
//  Global variables
//----------------------------------------------------------------------

// statistics output level
IMPKMEANSEXPORT extern StatLev            kmStatLev;

// standard output stream
IMPKMEANSEXPORT extern std::ostream*            kmOut;

// error output stream
IMPKMEANSEXPORT extern std::ostream*            kmErr;

// input stream
IMPKMEANSEXPORT extern std::istream*            kmIn;

//----------------------------------------------------------------------
//  Printing utilities
//----------------------------------------------------------------------

IMPKMEANSEXPORT void kmPrintPt(                        // print a point
  KMpoint            p,                  // the point
  int                  dim,                           // the dimension
  bool            fancy = true);            // print plain or fancy?

//! Log a point depending on log level (see base/log.h)
IMPKMEANSEXPORT void kmLogPt
( base::LogLevel ll,
  KMpoint            p,                  // the point
  int                  dim,                           // the dimension
  bool            fancy = true);            // print plain or fancy?

IMPKMEANSEXPORT void kmPrintPts(                  // print points
  std::string            title,                  // name of point set
  KMpointArray      pa,                  // the point array
  int                  n,                  // number of points
  int                  dim,                  // the dimension
  bool            fancy = true);            // print plain or fancy?

//! Log points depending on log level (see base/log.h)
IMPKMEANSEXPORT void kmLogPts
( base::LogLevel ll,
  std::string            title,                  // name of point set
  KMpointArray      pa,                  // the point array
  int                  n,                  // number of points
  int                  dim,                  // the dimension
  bool            fancy = true);            // print plain or fancy?




//----------------------------------------------------------------------
//  Utility function declarations
//----------------------------------------------------------------------

IMPKMEANSEXPORT void kmError(                        // error routine
  const std::string      &msg,                  // error message
  KMerr            level);                  // abort afterwards

IMPKMEANSEXPORT void kmExit(int x = 0);                 // exit the program

IMPKMEANS_END_INTERNAL_NAMESPACE

#endif /* IMPKMEANS_INTERNAL_KMEANS_H */
