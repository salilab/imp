/**
 *  \file domino/assignment_tables.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ASSIGNMENT_TABLES_H
#define IMPDOMINO_ASSIGNMENT_TABLES_H


#include "particle_states.h"
#include "subset_filters.h"
#include "Assignment.h"
#include "Subset.h"
#include "domino_config.h"
#include "domino_macros.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/internal/map.h>
#include <boost/pending/disjoint_sets.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

IMPDOMINO_BEGIN_NAMESPACE
class DominoSampler;

/** The base class for classes that create Assignments, one per
    subset. The main method of interest is get_assignments()
    which returns a Assignments containing the valid states.
*/
class IMPDOMINOEXPORT AssignmentsTable: public Object {
 public:
  AssignmentsTable(std::string name= "SubsetStatesTable %1%"): Object(name){}
  virtual void fill_assignments(const Subset &s,
                                AssignmentContainer *ac) const=0;
  ~AssignmentsTable();
};

IMP_OBJECTS(AssignmentsTable, AssignmentsTables);


/** Enumerate states based on provided ParticleStates
    objects.

    The produced states are filtered using the provided
    SubsetFilterTable objects. Branch and bound is used
    to try to make this process more efficient. To do that
    the SubsetFilterTable::get_strength() method is used
    to order the particles from most restricted to least
    restricted.
*/
class IMPDOMINOEXPORT BranchAndBoundAssignmentsTable:
  public AssignmentsTable {
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
/* MSVC/Sun gcc appears confused by a friend class in the anonymous namespace */
public:
  Pointer<ParticleStatesTable> pst_;
  SubsetFilterTables sft_;
  unsigned int max_;
#endif
 public:
  BranchAndBoundAssignmentsTable(ParticleStatesTable* pst,
                                  const SubsetFilterTables &sft
                                  = SubsetFilterTables(),
                                  unsigned int max
                                  = std::numeric_limits<unsigned int>::max());
  IMP_ASSIGNMENTS_TABLE(BranchAndBoundAssignmentsTable);
};


/** Store a map of Assignments objects and return them on demand. This table
    should be used when each subset is sampled using some other protocol
    (eg Monte Carlo or molecular dynamics) and those states are then fed
    in to domino.
*/
class IMPDOMINOEXPORT ListAssignmentsTable: public AssignmentsTable {
  IMP::internal::Map<Subset, Assignments> states_;
 public:
  ListAssignmentsTable(std::string name="ListSubsetStatesTable %1%");
  /** There must not be any duplicates in the list */
  void set_assignments(const Subset &s, const Assignments &lsc) {
    IMP_IF_CHECK(USAGE) {
      Assignments l= lsc;
      std::sort(l.begin(), l.end());
      IMP_USAGE_CHECK(std::unique(l.begin(), l.end())== l.end(),
                      "There are duplicated subset states in the passed list");
    }
    states_[s]=lsc;
  }
  IMP_ASSIGNMENTS_TABLE(ListAssignmentsTable);
};


/** Return the order computed for the particles in the subset to be used for
    enumeration. This function is there in order to expose internal
    functionality for easier testing and should not be depended upon.
*/
IMPDOMINOEXPORT Ints get_order(const Subset &s,
                               const SubsetFilterTables &sft);

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_ASSIGNMENT_TABLES_H */
