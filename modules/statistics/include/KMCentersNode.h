/**
 * \file KMCentersNode.h \brief generic node of the kc-tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_KM_CENTERS_NODE_H
#define IMPSTATISTICS_KM_CENTERS_NODE_H

#include "KMData.h"
#include "KMRectangle.h"
#include "KMCenters.h"
IMPSTATISTICS_BEGIN_NAMESPACE
//! Generic KMCentersTree node
/** Nodes in kc-trees are of two types, splitting nodes which contain
splitting information (a splitting hyperplane orthogonal to one of the
coordinate axes) and leaf nodes which contain point information (an array of
points stored in a bucket).
\unstable{KMCentersNode}
*/
class IMPSTATISTICSEXPORT KMCentersNode {
public:
  KMCentersNode(){}
    //!Constructor
    /**
\param[in] the bounding box of the points
     */
  KMCentersNode(const KMRectangle &bb, KMCenters *centers,int level);
 virtual ~KMCentersNode();
  //! Get the number of nodes in this subtree
  /**
  /note we assume that the number of data points in the leafs is 1
   */
 int get_subtree_size() { return 2*n_data_ - 1; }
 int get_number_of_data_points() { return n_data_; }

 KMPoint get_mid_point() const;
 KMRectangle &get_bounding_box() //TODO - should be const ?
   {  return bnd_box_;  }
 //! Compute sums of points
 virtual void compute_sums()=0;
 const KMPoint * get_sums() const {return &sum_;}
 double get_sum_sq() const {return sum_sq_;}
 //! Compute neighbors for centers - TODO should cands be const & ?
  virtual void get_neighbors(const std::vector<int> &cands,
       KMPointArray *sums, KMPoint *sum_sqs,std::vector<int> *weights)=0;
 //! get assignments for leaf node
 virtual void get_assignments(const std::vector<int> &cands,
        std::vector<int> &close_center)=0;
 // sample a center point c
 virtual KMPoint sample_center()=0;
 //Print node
 virtual void show(std::ostream&out=std::cout) const =0;
protected:
  //! Find the center closest to the middle of the bounding box of the node
  /**
   \param[in] cands a set of center indexes, one of which should be the closest
  */
  int mid_center(const std::vector<int> &cands);
  //! Determine if a candidate center is close enough
  /** \param[in] candidate_centers_inds the indexes of the candidate centers
    \param[in] close_centers_inds returned subset of candidate_centers_inds
               that are close enough to the bounding box of the node according
               to the following cretiria.
  A candidate center is close enough to the bounding box if b is closer to the
  candidate center to the middle center. b is the vertex of the bounding box
  that it closest of the candidate center. To excelerate the calculate we make
  use of the following observation: A point p is closer c than to c' if and only
  if (p-c).(p-c) < (p-c').(p-c') -> (c-c').(c-c') < 2(p-c').(c-c').
  */
  void compute_close_centers(
   const std::vector<int> &candidate_centers_inds,
   std::vector<int> *close_centers_inds);

  void post_neighbor(KMPointArray *sums, KMPoint *sum_sqs,
    std::vector<int> *weights,int center_ind);

  void post_one_neighbor(KMPointArray *sums, KMPoint *sum_sqs,
        std::vector<int> *weights,int center_ind, const KMPoint &p);
  int n_data_; // number of data points associated with the node
  KMPoint sum_; //sum of points for each dimension
  double  sum_sq_; // sum of squares
  KMRectangle  bnd_box_; // bounding box for cell
  KMCenters *centers_; //all of the centers
  int level_;
};

IMPSTATISTICS_END_NAMESPACE
#endif  /* IMPSTATISTICS_KM_CENTERS_NODE_H */
