/**
 * \file KMCenetersNode.h \brief leaf node of the kc-tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_KM_CENTERS_NODE_LEAF_H
#define IMPDOMINO_KM_CENTERS_NODE_LEAF_H

#include "KMCentersNode.h"
#include "KMData.h"
#include "KMRectangle.h"
IMPDOMINO_BEGIN_NAMESPACE
/** Leaf nodes of the kc-tree store the set of points associated with this
    bucket, stored as an array of point indices.  These are indices in the
    array points, which resides with the root of the kc-tree.
   We also store the number of points that reside in this bucket.
*/
class IMPDOMINOEXPORT KMCentersNodeLeaf: public KMCentersNode
{

public:
  KMCentersNodeLeaf(){}
  //! Constractor
  /**
/param[in] bb the bounding rectangle of the points that are associated
              with the node
/param[in] centers a pointer to all of the centers
/param[in] data_inds indexes of data points that are associated with the node
/param[in]
/param[in]
   */
  KMCentersNodeLeaf(int level,const KMRectangle &bb, KMCenters *centers,
    const std::vector<int> &data_inds) :  KMCentersNode(bb,centers,level) {
    IMP_LOG(VERBOSE, "add a new center node leaf with " <<
            data_inds.size() << " points" <<std::endl);
   IMP_assert(data_inds.size()>=1,
              "a leaf node should contain at least one point");
   for (unsigned int i=0;i<data_inds.size();i++) {
     data_ps_.push_back(data_inds[i]);}
   n_data_ = data_ps_.size();
 }

  //!Compute sums
  /**  Computes the sums and the sum of squares of points associated with
       the node. The sum of squares is the sum of dot products of each
       point with itself.
  */
 void compute_sums();
 //! Compute neighbors for centers
 void get_neighbors(const std::vector<int> &cands,KMPointArray *sums,
 KMPoint *sum_sqs,std::vector<int> *weights);
   //! Get assignments for leaf node
 void get_assignments(const std::vector<int> &cands,
                      std::vector<int> &close_center);

    //! Sample a center point c
 KMPoint sample_center();
 void show(std::ostream&out=std::cout) const ;
protected:
  std::vector<int> data_ps_; //the indexes of data points
                             //which are part of the leaf
};
IMPDOMINO_END_NAMESPACE
#endif  /* IMPDOMINO_KM_CENTERS_NODE_LEAF_H */
