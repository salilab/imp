/**
 * \file KMCentersNodeSplit.h
 * \brief a split node in the kc-tree with two children
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_CENTERS_NODE_SPLIT_H
#define IMPSTATISTICS_INTERNAL_KM_CENTERS_NODE_SPLIT_H

#include "KMCentersNode.h"
#include "KMData.h"
#include "KMRectangle.h"
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

//!  kc-tree splitting node.
/** Splitting nodes contain a cutting dimension and a cutting value. These
    indicate the axis-parellel plane which subdivide the box for this node.
    The extent of the bounding box along the cutting dimension is maintained
    (this is used to speed up point to box distance calculations) [we do not
    store the entire bounding box since this may be wasteful of space in
    high dimensions]. We also store pointers to the 2 children.
    \unstable{KMCentersNodeSplit}
*/
class IMPSTATISTICSEXPORT KMCentersNodeSplit : public KMCentersNode {
 public:
  KMCentersNodeSplit() {}
  //!Constractor
  /**
\param[in] dim the dimension of the points
\param[in] bb the bounding box
\param[in] cd the cutting dimension
\param[in] cv the cutting value
     */
  KMCentersNodeSplit(int level, KMRectangle &bb, KMCenters *centers, int cd,
                     double cv, double lv, double hv,
                     KMCentersNode *lc = nullptr, KMCentersNode *hc = nullptr)
      : KMCentersNode(bb, centers, level) {
    cut_dim_ = cd;
    cut_val_ = cv;
    cd_bnds_[0] = lv;   // lower bound for rectangle
    cd_bnds_[1] = hv;   // upper bound for rectangle
    children_[0] = lc;  // left child
    children_[1] = hc;  // right child
  }
  virtual ~KMCentersNodeSplit();
  //! Compute the sums of a split node.
  /** The sums of such a node derive from the children.
      The sum on a specific dimension is the sum of the sums of the children
      on that dimension.
   */
  void compute_sums();
  //! Compute neighbors for centers
  void get_neighbors(const Ints &cands, KMPointArray *sums, KMPoint *sum_sqs,
                     Ints *weights);
  void get_assignments(const Ints &cands, Ints &close_center);
  /** Let m denote the number of data points
      descended from this node.  Then with probability 1/(2m-1), this cell is
      chosen.  Otherwise, let mL and mR denote the number of points associated
      with the left and right subtrees, respectively. We sample from the left
      subtree with probability (2mL-1)/(2m-1) and sample from the right subtree
      with probability (2mR-1)/(2m-1).

      The rationale is that, assuming there is exactly one point per leaf node,
      a subtree with m points has exactly 2m-1 total nodes. (This should be
      true for this implementation, but it depends in general on the fact that
      there is exactly one point per leaf node.) Hence, the root should be
      sampled with probability 1/(2m-1), and the subtrees should be sampled
      with the given probabilities.
    */
  KMPoint sample_center();

  //! Print node
  void show(std::ostream &out = std::cout) const;

 protected:
  int cut_dim_;                 // dim orthogonal to cutting plane
  double cut_val_;              // location of cutting plane
  double cd_bnds_[2];           // lower and upper bounds of
                                // rectangle along cut_dim
  KMCentersNode *children_[2];  // left and right children
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_CENTERS_NODE_SPLIT_H */
