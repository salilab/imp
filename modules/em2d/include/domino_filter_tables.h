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
#include "IMP/em2d/ProjectionStates.h"
#include "IMP/em2d/ProjectionOverlapFilter.h"
#include "IMP/domino/Subset.h"
#include "IMP/domino/subset_filters.h"


IMPEM2D_BEGIN_NAMESPACE

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

IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_DOMINO_FILTER_TABLES_H */
