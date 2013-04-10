/**
 *  \file domino/interactive.cpp
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/domino/interactive.h>
#include <IMP/domino/internal/tree_inference.h>
#include <boost/random/uniform_int.hpp>


IMPDOMINO_BEGIN_NAMESPACE





void load_leaf_assignments(const Subset& subset,
                             AssignmentsTable *at,
                             AssignmentContainer *ac) {
  IMP_FUNCTION_LOG;
  internal::load_leaf_assignments(subset,
                                  at, nullptr, nullptr,
                                  ac);
}


void load_merged_assignments(const Subset &first_subset,
                             AssignmentContainer* first,
                             const Subset &second_subset,
                             AssignmentContainer* second,
                             const SubsetFilterTablesTemp &filters,
                             AssignmentContainer* ret) {
  IMP_FUNCTION_LOG;
  SubsetFilterTables ts(filters.begin(), filters.end());
  internal::load_merged_assignments(first_subset, first,
                                    second_subset, second,
                                    ts, nullptr, nullptr,
                                    std::numeric_limits<size_t>::max(),
                                    ret);
}



void
load_merged_assignments_random_order(const Subset &first_subset,
                                     AssignmentContainer* first,
                                     const Subset &second_subset,
                                     AssignmentContainer* second,
                                     const SubsetFilterTablesTemp
                                     &filters,
                                     unsigned int maximum_tries,
                                     AssignmentContainer* ret) {
  IMP_FUNCTION_LOG;
  SubsetFilterTables ts(filters.begin(), filters.end());
  IMP::Pointer<AssignmentContainer> retp(ret);
  IMP::OwnerPointer<AssignmentContainer> firstp(first),
    secondp(second);
  internal::EdgeData ed= internal::get_edge_data(first_subset, second_subset,
                                                 filters);
  Ints ii0 = internal::get_index(first_subset, ed.intersection_subset);
  Ints ii1 = internal::get_index(second_subset, ed.intersection_subset);
  Ints ui0 = internal::get_index(ed.union_subset, first_subset);
  Ints ui1 = internal::get_index(ed.union_subset, second_subset);
  Ints uii = internal::get_index(ed.union_subset, ed.intersection_subset);
  Assignments nd0a
    = first->get_assignments(IntRange(0, first->get_number_of_assignments()));
  Assignments nd1a
    = second->get_assignments(IntRange(0, second->get_number_of_assignments()));

  boost::uniform_int<> dist0(0, nd0a.size()-1);
  boost::uniform_int<> dist1(0, nd1a.size()-1);
  IMP_PROGRESS_DISPLAY("Merge " << first_subset << " and " << second_subset,
                       maximum_tries);
  for (unsigned int t = 0; t < maximum_tries; ++t) {
    unsigned int i = dist0(IMP::base::random_number_generator);
    unsigned int j = dist1(IMP::base::random_number_generator);
    Assignment nd0ae = internal::get_sub_assignment(nd0a[i], ii0);
    Assignment nd1ae = internal::get_sub_assignment(nd1a[j], ii1);
    if (nd1ae == nd0ae) {
      Assignment ss = internal::get_merged_assignment(ed.union_subset,
                                                      nd0a[i], ui0,
                                                      nd1a[j], ui1);
      bool ok = true;
      for (unsigned int k = 0; k < ed.filters.size(); ++k) {
        if (ed.filters[k]->get_is_ok(ss)) {
        } else {
          ok = false;
          break;
        }
      }
      if (ok) ret->add_assignment(ss);
    }
    IMP::base::add_to_progress_display();
  }
}
IMPDOMINO_END_NAMESPACE
