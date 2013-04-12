/**
 *  \file domino_filter_tables.h
 *  \brief Managing of projection overlap filter
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_DOMINO_FILTER_TABLES_H
#define IMPEM2D_DOMINO_FILTER_TABLES_H

#include "IMP/em2d/domino_particle_states.h"
#include "IMP/domino/Subset.h"
#include "IMP/domino/subset_filters.h"
#include "IMP/log.h"

IMPEM2D_BEGIN_NAMESPACE


/*! Table to create DistanceFilters on demand

 */
class IMPEM2DEXPORT DistanceFilterTable : public domino::SubsetFilterTable {
protected:
    domino::Subset my_subset_;
    Pointer<domino::ParticleStatesTable> ps_table_;
    double max_distance_; // max distance tolerated between the particles

public:

    DistanceFilterTable(const domino::Subset &subset_to_act_on,
            domino::ParticleStatesTable *ps_table,
            double max_distance) :
    my_subset_(subset_to_act_on), ps_table_(ps_table),
    max_distance_(max_distance) {
    };

    void show(std::ostream &out = std::cout) const {
        out << "DistanceFilterTable" << std::endl;
    };

    IMP_SUBSET_FILTER_TABLE(DistanceFilterTable);
};
IMP_OBJECTS(DistanceFilterTable, DistanceFilterTables);

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_DOMINO_FILTER_TABLES_H */
