/**
 *  \file example/ExampleSubsetFilterTable.h
 *  \brief A Score on the distance between a pair of particles.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPEXAMPLE_EXAMPLE_SUBSET_FILTER_TABLE_H
#define IMPEXAMPLE_EXAMPLE_SUBSET_FILTER_TABLE_H

#include "example_config.h"
#include <IMP/domino/subset_filters.h>
#include <IMP/domino/domino_macros.h>

IMPEXAMPLE_BEGIN_NAMESPACE

//! A dumb filter that forces the ith particle to be in the ith state.
/** The source code is as follows:
    \include ExampleSubsetFilterTable.h
    \include ExampleSubsetFilterTable.cpp
*/
class IMPEXAMPLEEXPORT ExampleSubsetFilterTable :
  public domino::SubsetFilterTable
{
  std::map<Particle*, int> index_;
public:
  ExampleSubsetFilterTable(const ParticlesTemp &order);
  IMP_SUBSET_FILTER_TABLE(ExampleSubsetFilterTable);
};

IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_EXAMPLE_SUBSET_FILTER_TABLE_H */
