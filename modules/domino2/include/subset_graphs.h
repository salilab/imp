/**
 *  \file domino2/subset_graphs.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_GRAPHS_H
#define IMPDOMINO2_SUBSET_GRAPHS_H

#include "particle_states.h"
#include "Subset.h"


IMPDOMINO2_BEGIN_NAMESPACE

/** A directed graph on subsets of vertices. Each vertex is
    named with an Subset.
 */
IMP_GRAPH(SubsetGraph, undirected, Subset, int);
//! Gets all of the Subsets of a SubsetGraph
IMPDOMINO2EXPORT Subsets get_subsets(const SubsetGraph &g);


//! A base class for generating the subset graph from the restraints
/** Currently, for DominoSampler, the returned graph must be a
    tree which satisfies the junction tree property.
**/
class IMPDOMINO2EXPORT SubsetGraphTable: public Object {
 public:
  SubsetGraphTable();
  virtual SubsetGraph get_subset_graph(ParticleStatesTable *pst) const=0;
};

IMP_OBJECTS(SubsetGraphTable, SubsetGraphTables);

//! Compute the junction tree
/** The passed restraint set implies the interaction
    graph that is used to generate the junction tree. The
    restraints in this set might be all of them
    (eg Model::get_root_restraint_set()), but it could just be
    a subset if you want to use some restraints for filtering only.
 */
class IMPDOMINO2EXPORT JunctionTreeTable: public SubsetGraphTable {
  Pointer<RestraintSet> rs_;
 public:
  JunctionTreeTable(RestraintSet *rs);
  IMP_SUBSET_GRAPH_TABLE(JunctionTreeTable);
};

IMP_OBJECTS(JunctionTreeTable, JunctionTreeTables);

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_GRAPHS_H */
