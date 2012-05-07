/**
 *  \file KMeansWrapper.h
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
#include "IMP/Pointer.h"
#include <cstdlib>      // C standard includes
#include <iostream>     // C++ I/O
#include <string>     // C++ strings
#include <string.h>                     // strcmp

IMPKMEANS_BEGIN_NAMESPACE

/** different k-means algorithm variants that are
    implemented in the library, see also
    http://www.cs.umd.edu/~mount/Projects/KMeans/
*/
enum KM_ALG_TYPE
{
  KM_LLOYDS = 1,
  KM_LOCAL_SWAP = 2,
  KM_LOCAL_EZ_HYBRID = 3,
  KM_HYBRID = 4
};


/** Class that wraps and provides an interface to the K-means
    library by David Mount (GPL license), downloaded and adapted
    to IMP from http://www.cs.umd.edu/~mount/Projects/KMeans/

    \untested{KMeansWrapper}
    \unstable{KMeansWrapper}
 */
class IMPKMEANSEXPORT KMeansWrapper {
 public:
  /**
     Initialize the KMeansWrapper object with data from fname_data,
     assuming input data of dimension dim

     @param[in] fname_data Input filename. Input is assumed to be textual,
                whitespace separated
     @param[in] dim Dimension of points
     @param[in] max_nPts Maximal number of points to be read from file
   */
  KMeansWrapper(const std::string& fname_data, int dim, int max_nPts);

  /**
     Execute a kmeans algorithm variant on the data points stored.

     @param[in] k number of clusters
     @param[in] alg_type The k-means algorithm variant to use
     @param[in] stages Number of k-means iterations
   */
  void execute(int k, KM_ALG_TYPE alg_type = KM_LLOYDS, int stages = 100);

  /** Returns the i'th center
      Must be called only following a succesful execute() invokation

      @param[in] i Center number in range (1,...,k)
   */
  std::vector<double> getCenter(int i) const;


 private:

  // print a point
  // out - stream for printing the point
  // p - the point
  void printPt(std::ostream& out, const internal::KMpoint&  p);

  // read a point (using dimension from dataPts_)
  bool readPt(std::istream& in, internal::KMpoint& p);

  // read up to max_nPts from a file using dimension define in dataPts_
  // returns true if successful
  bool readDataPts(std::istream &in, int max_nPts);

  // print final summary using stored data and centers after execution
  void printSummary(const internal::KMlocal&  theAlg);   // the algorithm

  /** was the object initialized succesfuly */
  bool is_initialized() { return is_initialized_; }

  /** was k-means executed succesfuly */
  bool is_executed() { return is_executed_; }

 private:

  // was the object initialized succesfuly
  bool is_initialized_;

  // was k-means executed succesfully
  bool is_executed_;

  //----------------------------------------------------------------------
  //  Termination conditions
  //  These are explained in the files internal/KMterm.h and internal/KMlocal.h
  //  Unless you are into fine tuning, don't worry about changing these.
  //----------------------------------------------------------------------
  internal::KMterm term_;

  // data points
  internal::KMdata dataPts_;

  // actual number of valid data points
  int nPts_;

  // the center points from a clustering execition
  IMP::Pointer<internal::KMfilterCenters> pCenters_;

};

IMPKMEANS_END_NAMESPACE

#endif  /* IMPKMEANS_KMEANS_WRAPPER_H */
