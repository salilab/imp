/**
 *  \file domino/HeapAssignmentContainer.h
 *  \brief heap container for domino
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPDOMINO_HEAP_ASSIGNMENT_CONTAINER_H
#define IMPDOMINO_HEAP_ASSIGNMENT_CONTAINER_H

#include "domino_config.h"
#include "subset_filters.h"
#include "Assignment.h"
IMPDOMINO_BEGIN_NAMESPACE
namespace {
  bool comp_first_larger_than_second(
                         const std::pair<Assignment,double> &a,
                         const std::pair<Assignment,double> &b) {
    return a.second > b.second;
  }
}
/** Store a set of k top scoring assignemnts
 */
class IMPDOMINOEXPORT HeapAssignmentContainer: public AssignmentContainer {
  std::vector<std::pair<Assignment,double> > d_;
  unsigned int k_; // max number of assignments (heap size)
  Pointer<RestraintScoreSubsetFilter> rssf_;//to score candidate assignments
 public:
  HeapAssignmentContainer(unsigned int k,
                          RestraintScoreSubsetFilter *rssf,
                          std::string name="HeapAssignmentsContainer %1%");
  IMP_ASSIGNMENT_CONTAINER(HeapAssignmentContainer);
};

IMPDOMINO_END_NAMESPACE
#endif /* IMPDOMINO_HEAP_ASSIGNMENT_CONTAINER_H */
