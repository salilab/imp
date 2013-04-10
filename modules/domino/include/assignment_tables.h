/**
 *  \file IMP/domino/assignment_tables.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_ASSIGNMENT_TABLES_H
#define IMPDOMINO_ASSIGNMENT_TABLES_H


#include "particle_states.h"
#include "subset_filters.h"
#include "Assignment.h"
#include "Subset.h"
#include <IMP/domino/domino_config.h>
#include "assignment_containers.h"
#include "domino_macros.h"
#include <IMP/Sampler.h>
#include <IMP/macros.h>
#include <IMP/base/map.h>
#include <boost/pending/disjoint_sets.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

IMPDOMINO_BEGIN_NAMESPACE
class DominoSampler;

/** The base class for classes that create Assignments, one per
    subset. The main method of interest is load_assignments()
    which enumerates the assignments and loads them into an AssignmentContainer.
*/
class IMPDOMINOEXPORT AssignmentsTable: public IMP::base::Object {
 public:
  AssignmentsTable(std::string name= "SubsetStatesTable %1%"): Object(name){}
  virtual void load_assignments(const Subset &s,
                                AssignmentContainer *ac) const=0;
  ~AssignmentsTable();
};

IMP_OBJECTS(AssignmentsTable, AssignmentsTables);


/** The produced states are filtered using the provided
    SubsetFilterTable objects. The assignments are enumerated
    and filtered in a straight forward manner.
*/
class IMPDOMINOEXPORT SimpleAssignmentsTable:
  public AssignmentsTable {
  Pointer<ParticleStatesTable> pst_;
  SubsetFilterTables sft_;
  unsigned int max_;
 public:
  SimpleAssignmentsTable(ParticleStatesTable* pst,
                         const SubsetFilterTables &sft
                         = SubsetFilterTables(),
                         unsigned int max
                         = std::numeric_limits<unsigned int>::max());
  IMP_ASSIGNMENTS_TABLE(SimpleAssignmentsTable);
};


/** The produced states are filtered using the provided
    SubsetFilterTable objects. The assignments are enumerated
    and filtered by recursively dividing the subset in half.
*/
class IMPDOMINOEXPORT RecursiveAssignmentsTable:
  public AssignmentsTable {
  Pointer<ParticleStatesTable> pst_;
  SubsetFilterTables sft_;
  unsigned int max_;
 public:
  RecursiveAssignmentsTable(ParticleStatesTable* pst,
                         const SubsetFilterTables &sft
                         = SubsetFilterTables(),
                         unsigned int max
                         = std::numeric_limits<unsigned int>::max());
  IMP_ASSIGNMENTS_TABLE(RecursiveAssignmentsTable);
};


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
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  IMP::base::map<Particle*, ParticlesTemp> rls_;
#endif
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
  IMP::base::map<Subset,
                          IMP::OwnerPointer<AssignmentContainer> >
    states_;
 public:
  ListAssignmentsTable(std::string name="ListSubsetStatesTable %1%");
  /** There must not be any duplicates in the list */
  void set_assignments(const Subset &s,  AssignmentContainer *lsc) {
    states_[s]=lsc;
  }
  IMP_ASSIGNMENTS_TABLE(ListAssignmentsTable);
};


/** Return the order computed for the particles in the subset to be used for
    enumeration. This function is there in order to expose internal
    functionality for easier testing and should not be depended upon.
*/
IMPDOMINOEXPORT ParticlesTemp get_order(const Subset &s,
                               const SubsetFilterTables &sft);

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_ASSIGNMENT_TABLES_H */
