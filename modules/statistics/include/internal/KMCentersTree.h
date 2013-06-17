/**
 *  \file KMCentersTree.h   \brief A tree that handles point partition
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_CENTERS_TREE_H
#define IMPSTATISTICS_INTERNAL_KM_CENTERS_TREE_H

#include "KMData.h"
#include "KMCenters.h"
#include "KMCentersNode.h"
#include "KMCentersNodeLeaf.h"
#include "KMCentersNodeSplit.h"

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

//! A data structure for efficeint assignment of points to clusters
/**
   \unstable{KMCentersTree}
 */
class KMCentersTree {
 public:
  //!  Constructor
  /**
   \param[in] data_points all data points
   \param[in] centers the centers
   /parm[in]  bb_lo Bounding box left-bottom coordinates (default: compute
   bounding box from points).
   /parm[in]  bb_hi Bounding box right_upper coordiantes (default: compute
   bounding box from points).
   /note As long as the number of points is nonzero, or if a bounding box
         is provided, then the constructor will build a tree with at least one
         node (even an empty leaf). Otherwise, it returns with a null tree.
  */
  KMCentersTree(KMData *data_points, KMCenters *centers,
                KMPoint *bb_lo = nullptr, KMPoint *bb_hi = nullptr);

  //! Compute the neighboring data points of each center
  /** This is the heart of the filter-based k-means algorithm.
  \param[in] sums     an array of center sums to update
  \param[in] sums_sqs an array of center sums of squares to update
  \param[in] weights  the number of points associated with each center
  /note From these parameters the final centroid and distortion
  (mean squared error) can be computed. This is done by determining the set of
  candidates for each node in the tree.
  When the number of center candidates for a node is equal to 1 (it cannot be 0)
  then all of the points in the subtree rooted at this node are assigned as
  neighbors to this center.  This means that the centroid and weight for this
  cell is added into the neighborhood centroid sum for this center.  If this
  node is a leaf, then we compute (by brute-force) the distance from each
  candidate to each data point, and assign the data point to the closest
  center.
  */
  void get_neighbors(KMPointArray *sums, Floats *sum_sqs, Ints *weights);

  //! Compute assignment of data points to closest center
  /** A structural copy of the procedure get_neighbors, but rather than
  incrementing the various sums and sums of squares, it simply records
  the assignment of each data point to its closest center. Unlike the
  filtering search, when only one candidate remains, it does not stop the
  search, but continues to traverse all the leaves descended from this node in
  order to perform the assignments.
  \param[out] close_center  will contain the closest center index for
              each of the data points
  */
  void get_assignments(Ints &close_center);

  ~KMCentersTree();
  //! sample a center point c
  /** This implements an approach suggested by Matoushek for sampling a
      center point.
      1. A node of the tree is selected at random.
         1.1 If this is an interior node, a point is sampled uniformly from a 3x
             expansion of its bounding rectangle.
         1.2 If the node is a leaf, then a data point is sampled at random from
             the associated bucket.
  */

  //!  Sample a center point
  /** A node of is selected at random. If this is an interior node, a point is
  sampled uniformly from a 3x expansion of the cell about its center.
  If the node is a  leaf, then a data point is sampled at random from the
  associated bucket.
  */
  KMPoint sample_center();
  void show(std::ostream &out = std::cout);

 protected:
  //! Initializes the basic tree elements (without building the tree)
  /**
  \param[in] bb_lo bounding box low point (optional)
  \param[in] bb_hi bounding box high point (optional)
  \param[in] p_id point indices (optional)
  /note If p_id is nullptr then the constructor should initialize the array of
        indices
  */
  void skeleton_tree(const Ints &pi, KMPoint *bb_lo = nullptr,
                     KMPoint *bb_hi = nullptr);
  //!Recursive construction of the tree from a set of points.
  /**
  \param[in] pa the points
  \param[in] pidx point indices to store in subtree
  \param[in] n number of points
  \param[in] dim the dimension of space
  \param[in] bnd_box bounding box for current node
  /note The construction is based on a standard algorithm for constructing
  the kc-tree (see Friedman, Bentley, and Finkel, ``An algorithm for finding
  best matches in logarithmic expected time,'' ACM Transactions on Mathematical
  Software, 3(3):209-226, 1977).  The procedure operates by a simple
  divide-and-conquer strategy, which determines an appropriate orthogonal
  cutting plane, and splits the points. When the number of points falls
  below 1, we simply store the points in a leaf node's bucket.
  This procedure selects a cutting dimension and cutting value, partitions
  ps about these values, and returns the number of points on the low side
  of the cut. Note that this procedure is not only used for constructing full
  trees, but is also used by the insertion routine to rebuild a subtree.
  */
  KMCentersNode *build_tree(int start_ind, int end_ind, int level);

  //! split a set of points about a cutting plane along a given cutting
  // dimension.
  /**
  /note split the points whose indexes are stored in p_id [start_ind,end_ind]
  \param[in] start_ind the start index
  \param[in] end_ind the last index
  \param[in] p_id point indexes
  \param[in] dim the dimension to split upon
  \param[in] cv cutting value
  \param[out] a pair (a,b) of breaks such that ps[start_ind,...,a-1] < cv,
              ps[a,b-1]=cv and ps[b,end_ind] > cv
  /note the partition is stored in p_id
   */
  std::pair<int, int> split_by_plane(int start_ind, int end_ind, int dim,
                                     double cv);

  //! Split the points according to value of the middle point of
  //! a certain dimension
  /**
  /note Use the midpoint rule by bisecting the longest side. If there are ties,
  the dimension with the maximum spread is selected.
  If, however, the midpoint split produces a trivial split (no points on one
  side
  of the splitting plane) then we slide the splitting (maintaining its
  orientation) until it produces a nontrivial split.  For example, if the
  splitting plane is along the x-axis, and all the data points have x-coordinate
  less than the x-bisector, then the split is taken along the maximum
  x-coordinate
  of the data points. Intuitively, this rule cannot generate trivial splits,
  and hence avoids midpt_split's tendency to produce trees with a very large
  number of nodes.
  \param[in] start_ind the index of the first point in p_id_
  \param[in] end_ind the index of the last point in p_id_
  \param[out] cut_dim the index of cutting dimension
  \param[out] cut_val the cutting value
  \param[out] n_lo the number of points in the low side
  */
  void split_by_mid_point(int start_ind, int end_ind, int &cut_dim,
                          double &cut_val, int &n_lo);
  double get_value(int p_id, int dim) const;
  //! Find the bounding rectangle of points represented by
  //! data_points_[p_id_[i]] i in [start_ind,end_ind]
  KMRectangle *bounding_rectangle(int start_ind, int end_ind);
  //! Find the lenght of bounding rectangle of points
  //! represented by data_points_[p_id_[i]] i in [start_ind,end_ind]
  //! in a certain dimension
  double spread(int start_ind, int end_ind, int dim);
  //! Find the limits of the edge in the bounding rectangle of points
  //! represented by data_points_[p_id_[i]] i in [start_ind,end_ind] in
  //! a certain dimension
  std::pair<double, double> limits_along_dimension(int start_ind, int end_ind,
                                                   int dim);

  KMData *data_points_;  // all of the data points
  KMCenters *centers_;
  Ints p_id_;  // the indexes of the data points sorted by
  // the plane splitting algorithm
  KMCentersNode *root_;
  KMRectangle *bnd_box_;
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_CENTERS_TREE_H */
