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

/***********************     Typedefs     **************************/

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

typedef std::vector< std::vector<double> > VectorVectorDouble;

/***********************  Class Definition  **************************/

/** Class that wraps and provides an interface to the K-means
    library by David Mount (GPL license), downloaded and adapted
    to IMP from http://www.cs.umd.edu/~mount/Projects/KMeans/

    \untested{KMeansWrapper}
    \unstable{KMeansWrapper}
 */
class IMPKMEANSEXPORT KMeansWrapper {

  /***********************  Constructors  **************************/
 public:

  /**
     Initialize the KMeansWrapper object with data from fname_data,
     assuming input data of dimension dim

     @param[in] fname_data Input filename. Input is assumed to be textual,
                whitespace separated
     @param[in] dim Dimension of points
     @param[in] max_nPts Maximal number of points to be read from file
   */
  KMeansWrapper
    (const std::string& fname_data,
     int dim,
     unsigned int max_nPts);

  /** Empty constructor for all default initializations -
      object data is not considered initialized after this call
  */
  KMeansWrapper();

  /*********************** Public methods **************************/
 public:

  /**
     Execute a kmeans algorithm variant on the data points stored.

     @param[in] k number of clusters
     @param[in] alg_type The k-means algorithm variant to use
     @param[in] stages Number of k-means iterations
   */
  void execute(int k, KM_ALG_TYPE alg_type = KM_LLOYDS, int stages = 100);

  /**
     Add a data point for clustering.

     @param[in] p point to be added
  */
  void addDataPt(const std::vector<double>& p);

  /**
     Clears all data in object.
  */
  void clearData();

  /** Returns the i'th point in the dataset

      @param[in] i Center number in range (0,...,nPts-1)
   */
  const std::vector<double>& getDataPoint(unsigned int i) const;

  /** @return The number of data points */
  unsigned int getNDataPoints() const
  {
    return STLDataPts_.size();
  }

  /**
      Print the centers (assuming exectute() was applied)
   */
  void printCenters() const;

  /** Returns the i'th center
      Must be called only following a succesful execute() invokation

      @param[in] i Center number in range (0,...,k-1)
   */
  std::vector<double> getCenter(unsigned int i) const;


  /** @return The number of centers after a succeful execution */
  unsigned int getNCenters() const
  {
    assert( is_executed_ ); // TODO: exception?
    return pCenters_->getK();
  }

  /*********************** Private methods **************************/
 private:

  /** Updates the wrapped data pts structure from the internal 2D STL vector
      array.
      This method invalidates any prior information about clustering results,
      unless the data was already synced (in which case no sync was needed)
   */
  void syncKMDataPtsFromSTL();

  /**
     Read a point from a stream into p

     @param[in]  in    input stream to read from
     @param[out] p     output point
     @param[in]  dim   dimension of each data point

     @return false on error or EOF.
  */
  bool readPtFromStream
    (std::istream& in,
     std::vector<double>& p,
     unsigned int dim);

  /**
     Read up to max_nPts from a stream

     @param[in] in       input stream to read from
     @param[in] dim      dimension of each data point
     @param[in] max_nPts maximal number of points to read from stream
  */
  void readDataPtsFromStream
    (std::istream &in,
     unsigned int dim,
     unsigned int max_nPts);

  /**
   print a point

   @param[in] out stream for printing the point
   @param[in] p   the point
  */
  void printPtToStream
    (std::ostream& out,
     const std::vector<double>& p);

  // print final summary using stored data and centers after execution
  void printSummary(const internal::KMlocal&  theAlg);   // the algorithm

  /*********************** Private Variables **************************/
 private:

  // was k-means executed succesfully
  bool is_executed_;

  // The data points in STL format
  VectorVectorDouble STLDataPts_;

  // data points in wrapped internal::KMdata strcture
  // (should be synced from STLDataPts_ before usage)
  IMP::Pointer<internal::KMdata> pKMDataPts_;

  // was STL data updated to wrapped internal::KMdata points
  bool is_KM_data_synced_;

  // the center points from a clustering execition
  IMP::Pointer<internal::KMfilterCenters> pCenters_;

  //----------------------------------------------------------------------
  //  Termination conditions
  //  These are explained in the files internal/KMterm.h and internal/KMlocal.h
  //  Unless you are into fine tuning, don't worry about changing these.
  //----------------------------------------------------------------------
  internal::KMterm terminationConditions_;


}; /***********************  class KMeansWrapper  **************************/

IMPKMEANS_END_NAMESPACE

#endif  /* IMPKMEANS_KMEANS_WRAPPER_H */
