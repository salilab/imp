/**
 *  \file ProjectionOverlapFilterTable
 *  \brief Managing of projection overlap filter
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_DOMINO_FILTER_TABLES_H
#define IMPEM2D_DOMINO_FILTER_TABLES_H

#include "IMP/em2d/Image.h"
#include "IMP/em2d/domino_particle_states.h"
#include "IMP/domino/Subset.h"
#include "IMP/domino/subset_filters.h"
#include "IMP/log.h"


IMPEM2D_BEGIN_NAMESPACE

/*
class IMPEM2DEXPORT ProjectionOverlapFilterTable:
                      public domino::SubsetFilterTable {
protected:

  Pointer<Image> image_;
  Pointer<ProjectionStates> projection_states_;
  domino::Subset my_subset_;
public:

  ProjectionOverlapFilterTable(Image *image,
                               ProjectionStates *projection_states) :
                               image_(image),
                               projection_states_(projection_states) {};

  IMP_SUBSET_FILTER_TABLE(ProjectionOverlapFilterTable);

};
IMP_OBJECTS(ProjectionOverlapFilterTable,ProjectionOverlapFilterTables);

*/

/*! Table to create DistanceFilters on demand
  \param[in]
*/
class IMPEM2DEXPORT DistanceFilterTable: public domino::SubsetFilterTable {
protected:
  domino::Subset my_subset_;
  Pointer<domino::ParticleStatesTable> ps_table_;
  double max_distance_; // max distance tolerated between the particles

public:

  DistanceFilterTable(const domino::Subset &subset_to_act_on,
                      domino::ParticleStatesTable *ps_table,
                      double max_distance) :
                    my_subset_(subset_to_act_on), ps_table_(ps_table),
                      max_distance_(max_distance) {};

  void show(std::ostream &out = std::cout) const {
    out << "DistanceFilterTable" << std::endl;
  };

  IMP_SUBSET_FILTER_TABLE(DistanceFilterTable);
 };
IMP_OBJECTS(DistanceFilterTable, DistanceFilterTables);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_DOMINO_FILTER_TABLES_H */
