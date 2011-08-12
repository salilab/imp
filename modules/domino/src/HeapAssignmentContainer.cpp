/**
 *  \file domino/HeapAssignmentContainer.cpp
 *  \brief heap container for domino
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/HeapAssignmentContainer.h>
IMPDOMINO_BEGIN_NAMESPACE
inline unsigned int
HeapAssignmentContainer::get_number_of_assignments() const {
  return d_.size();
}

inline Assignment
HeapAssignmentContainer::get_assignment(unsigned int i) const {
  IMP_USAGE_CHECK(i < get_number_of_assignments(),
                  "Invalid assignment requested: " << i);
  return Assignment(d_[i].first);
}
HeapAssignmentContainer::HeapAssignmentContainer(
                               unsigned int k,
                               RestraintScoreSubsetFilter *rssf,
                               std::string name):
  AssignmentContainer(name), k_(k), rssf_(rssf) {}

void HeapAssignmentContainer::do_show(std::ostream &out) const {
  out<<"HeapAssignmentContainer: ";
  out << "number of assignments: " << get_number_of_assignments();
  out << ", max heap size: " << k_ << std::endl;
}

void HeapAssignmentContainer::add_assignment(Assignment a) {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (unsigned int i=0; i< get_number_of_assignments(); ++i) {
      IMP_INTERNAL_CHECK(get_assignment(i) != a,
                         "Assignment " << a << " already here.");
    }
  }
  double score=rssf_->get_score(a);
  //if small number of assignments, add this one
  if (d_.size()<k_) {
    d_.push_back(std::make_pair(a,score));
    std::sort_heap(d_.begin(),d_.end(),
                   comp_first_larger_than_second);
  }
  else {
    //add the configuration if is scores better
    if (score<d_[0].second) {
      //instead of pop just replace and head and sort
      d_[0]=std::make_pair(a,score);
      std::sort_heap(d_.begin(),d_.end(),
                     comp_first_larger_than_second);
    }
  }
  if (d_.size()%1000000 == 0) {
    std::cout<<"Current subset size:"<<d_.size()<<" : "<<a<<std::endl;
  }
}
IMPDOMINO_END_NAMESPACE
