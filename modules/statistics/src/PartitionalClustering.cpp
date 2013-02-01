/**
 *  \file Clustering.cpp   \brief Holds data points to cluster using k-means
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/PartitionalClustering.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/set.h>

IMPSTATISTICS_BEGIN_NAMESPACE
PartitionalClustering::~PartitionalClustering(){}


void validate_partitional_clustering(PartitionalClustering*pc,
                                     unsigned int n) {
  IMP_CHECK_VARIABLE(pc);
  IMP_CHECK_VARIABLE(n);
  IMP_IF_CHECK(base::USAGE) {
    IMP::base::set<int> used;
    for (unsigned int i=0; i< pc->get_number_of_clusters(); ++i) {
      Ints cluster= pc->get_cluster(i);
      for (unsigned int j=0; j< cluster.size(); ++j) {
        int e= cluster[j];
        IMP_USAGE_CHECK(used.find(e)== used.end(),
                        "Element " << e << " has been used twice: "
                        << Showable(pc));
        used.insert(e);
      }
    }
    IMP_USAGE_CHECK(used.size()==n, "Not all elements are clustered: "
                    << used.size() << " of " << n << " found: "
                    << Showable(pc));
  }
}
IMPSTATISTICS_END_NAMESPACE
