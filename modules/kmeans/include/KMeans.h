/**
 *  \file IMP/kmeans/KMeans.h
 *  \brief an interface to k-means open source library (stored internally)
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
*/

#ifndef IMPKMEANS_KMEANS_H
#define IMPKMEANS_KMEANS_H

#include <IMP/kmeans/kmeans_config.h>
#include "IMP/kmeans/internal/KMlocal.h"     // k-means algorithms
#include "IMP/kmeans/internal/KMdata.h"     // k-means algorithms
#include "IMP/kmeans/internal/KMterm.h"
#include "IMP/Pointer.h"
#include "IMP/base/doxygen_macros.h"
#include "IMP/base/object_macros.h"
#include "IMP/base/warning_macros.h"
#include "IMP/base/Object.h"
#include "IMP/base/types.h"
#include <cstdlib>      // C standard includes
#include <iostream>     // C++ I/O
#include <string>     // C++ strings
#include <string.h>                     // strcmp

IMPKMEANS_BEGIN_NAMESPACE

/***********************     Typedefs     **************************/

/** Different k-means algorithm variants that are
    implemented in the library, see also
    http://www.cs.umd.edu/~mount/Projects/KMeans/
*/
enum KM_ALG_TYPE
{
  /**
     Repeatedly applies Lloyd's algorithm with randomly sampled starting
     points.
  */
  KM_LLOYDS = 1,
  /**
     A local search heuristic, which works by performing swaps between existing
     centers and a set of candidate centers.
  */
  KM_LOCAL_SWAP = 2,
  /**
     A simple hybrid algorithm, which does one swap followed by some number of
     iterations of Lloyd's.
  */
  KM_LOCAL_EZ_HYBRID = 3,
  /**
     A more complex hybrid of Lloyd's and Swap, which performs some number of
     swaps followed by some number of iterations of Lloyd's algorithm. To
     avoid getting trapped in local minima, an approach similar to simulated
     annealing is included as well.
  */
  KM_HYBRID = 4
};

/***********************  Class Definition  **************************/

/** Class that wraps and provides an interface to the K-means
    library by David Mount (GPL license), downloaded and adapted
    to IMP from http://www.cs.umd.edu/~mount/Projects/KMeans/

    For a simple usage example, see
    modules/kmeans/examples/kmeans_example.py

    \untested{KMeans}
    \unstable{KMeans}
 */
class IMPKMEANSEXPORT
KMeans : public IMP::base::Object {

  /***********************  Constructors  **************************/
 public:

  /**
     Initialize the KMeans object with data from fname_data,
     assuming input data of dimension dim

     @param[in] fname_data Input filename. Input is assumed to be in
                textual (ascii) whitespace separated format, with a
                fixed number of columns dim.  Each row represents a
                single data point of fixed dimension dim.  For
                example, a file with three examples of dimension 4
                would look as follows:\n
                10.3  0.7  1.3 11.1\n
                2.1   1.5 20.1  0.2\n
                10.1  0.9  2.1 10.9
     @param[in] dim Dimension of each data point
     @param[in] max_nPts Maximal number of points to be read from file
   */
  KMeans
    (const std::string& fname_data,
     int dim,
     unsigned int max_nPts);

  /** Empty constructor for all default initializations -
      object data is not considered initialized after this call
  */
  KMeans();

  /************   Object virtual methods / destructor   ************/

  IMP_OBJECT_METHODS(KMeans);

  /*********************** Public methods **************************/
 public:

  /**
     Execute a kmeans algorithm variant on the data points stored.

     @param[in] k number of clusters
     @param[in] alg_type The k-means algorithm variant to use
                \see KM_ALG_TYPE
     @param[in] stages Number of k-means iterations
   */
  void execute
    (unsigned int k,
     KM_ALG_TYPE alg_type = KM_LLOYDS,
     int stages = 100);

  /**
     Add a data point for clustering.

     @param[in] p point to be added
  */
  void add_data_pt(const IMP::Floats& p);

  /**
     Clears all data in object.
  */
  void clear_data();

  /** Returns the i'th point in the dataset

      @param[in] i Center number in range (0,...,nPts-1)
   */
  const IMP::Floats& get_data_point(unsigned int i) const;

  /** @return The number of data points */
  unsigned int get_number_of_data_points() const
  {
    return STLDataPts_.size();
  }

  /**
      Print the centers (assuming exectute() was applied)
   */
  void print_centers() const;

  /** Returns the i'th center
      Must be called only following a succesful execute() invokation

      @param[in] i Center number in range (0,...,k-1)
   */
  IMP::Floats get_center(unsigned int i) const;

  /** Returns the cluster assignment of each data point */
  IMP::Ints get_assignments() const
    {
      // TODO: exception instead of assertion?
      assert(is_executed_);
      return centerAssignments_;
    }

  /** Returns the squared distance of each data point to its
      respective cluster center */
  IMP::Floats get_squared_distance_to_centers() const
    {
      // TODO: exception instead of assertion?
      assert(is_executed_);
      return ptsSqrDistToCenters_;
    }

  /** @return The number of centers after a succeful execution */
  unsigned int get_number_of_centers() const
  {
    assert( is_executed_ ); // TODO: exception?
    return pCenters_->getK();
  }

  /*********************** Private methods **************************/
 private:

  /** Updates the wrapped data pts structure from the internal 2D STL vector
      array (IMP::Float).
      This method invalidates any prior information about clustering results,
      unless the data was already synced (in which case no sync was needed)
   */
  void sync_KMdata_pts_from_STL();

  /**
     Read a point from a stream into p

     @param[in]  in    input stream to read from
     @param[out] p     output point
     @param[in]  dim   dimension of each data point

     @return false on error or EOF.
  */
  bool read_pt_from_stream
    (std::istream& in,
     IMP::Floats& p,
     unsigned int dim);

  /**
     Read up to max_nPts from a stream

     @param[in] in       input stream to read from
1     @param[in] dim      dimension of each data point
     @param[in] max_nPts maximal number of points to read from stream
  */
  void read_data_pts_from_stream
    (std::istream &in,
     unsigned int dim,
     unsigned int max_nPts);

  /**
   print a point

   @param[in] out stream for printing the point
   @param[in] p   the point
  */
  void print_pt_to_stream
    (std::ostream& out,
     const IMP::Floats& p);

  // print final summary using stored data and centers after execution
  void print_summary(const internal::KMlocal&  theAlg);   // the algorithm

  /*********************** Private Variables **************************/
 private:

  // was k-means executed succesfully
  bool is_executed_;

  // The data points in STL format
  IMP::FloatsList STLDataPts_;

  // data points in wrapped internal::KMdata strcture
  // (should be synced from STLDataPts_ before usage)
  IMP::Pointer<internal::KMdata> pKMDataPts_;

  // was STL data updated to wrapped internal::KMdata points
  bool is_KM_data_synced_;

  // the center points from a clustering execution
  IMP::Pointer<internal::KMfilterCenters> pCenters_;

  // cluster of each point
  IMP::Ints centerAssignments_;

  // data points squared distances from respective cluster centers
  IMP::Floats ptsSqrDistToCenters_;

  //----------------------------------------------------------------------
  //  Termination conditions
  //  These are explained in the files internal/KMterm.h and internal/KMlocal.h
  //  Unless you are into fine tuning, don't worry about changing these.
  //----------------------------------------------------------------------
  internal::KMterm terminationConditions_;

}; /***********************  class KMeans  **************************/

IMPKMEANS_END_NAMESPACE

#endif  /* IMPKMEANS_KMEANS_H */
