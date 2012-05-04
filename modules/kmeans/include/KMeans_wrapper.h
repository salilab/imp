/**
 *  \file KMeans_interface.h
 *  \brief an interface to k-means open source library (stored internally)
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
*/

#ifndef IMPKMEANS_KMEANS_WRAPPER_H
#define IMPKMEANS_KMEANS_WRAPPER_H

#include "kmeans_config.h"
#include "IMP/kmeans/internal/KMlocal.h"     // k-means algorithms
#include "IMP/kmeans/internal/KMdata.h"     // k-means algorithms
#include "IMP/kmeans/internal/KMterm.h"
#include <cstdlib>      // C standard includes
#include <iostream>     // C++ I/O
#include <string>     // C++ strings
#include <string.h>                     // strcmp

IMPKMEANS_BEGIN_NAMESPACE


//----------------------------------------------------------------------
// kmlsample
//
// This is a simple sample program for the kmeans local search on each
// of the four methods.  After compiling, it can be run as follows.
//
//   kmlsample [-d dim] [-k nctrs] [-max mpts] [-df data] [-s stages]
//
// where
//  dim   Dimension of the space (default = 2)
//  nctrs   The number of centers (default = 4)
//  mpts    Maximum number of data points (default = 1000)
//  data    File containing data points
//      (If omitted mpts points are randomly generated.)
//  stages    Number of stages to run (default = 100)
//
// Results are sent to the standard output.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Main program
//----------------------------------------------------------------------


class IMPKMEANSEXPORT KMeans_wrapper {
 public:
  KMeans_wrapper(const std::string& fname_data, int dim, int max_nPts);

  void execute(int k, int stages);

 private:

  // print a point
  void printPt(std::ostream& out, const internal::KMpoint&  p);   // the point

  // read a point (using dimension from dataPts_)
  bool readPt(std::istream& in, internal::KMpoint& p);

  // read up to max_nPts from a file using dimension define in dataPts_
  // returns true if successful
  bool readDataPts(std::istream &in, int max_nPts);

  // print final summary for a set of centers and algorithm
  void printSummary(
                    const internal::KMlocal&  theAlg,   // the algorithm
                    internal::KMfilterCenters&  centers);

  bool good() { return good_; }

 private:

  //is everything ok with the state of the object
  bool good_;

  //----------------------------------------------------------------------
  //  Termination conditions
  //  These are explained in the files internal/KMterm.h and internal/KMlocal.h
  //  Unless you are into fine tuning, don't worry about changing these.
  //----------------------------------------------------------------------
  internal::KMterm term_;

  internal::KMdata dataPts_; // data points

  int nPts_; // actual number of valid points

};

IMPKMEANS_END_NAMESPACE

#endif  /* IMPKMEANS_KMEANS_WRAPPER_H */
