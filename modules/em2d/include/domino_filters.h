/**
 *  \file ProjectionOverlapFilter.h
 *  \brief SubsetFilter for checking overlap between projections and images
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPEM2D_DOMINO_FILTERS_H
#define IMPEM2D_DOMINO_FILTERS_H

#include "IMP/em2d/Image.h"
#include "IMP/em2d/image_processing.h"
#include "IMP/em2d/ProjectionStates.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/domino/subset_filters.h"
#include "IMP/domino/Assignment.h"
#include "IMP/Pointer.h"
#include "IMP/base_types.h"
#include <iostream>

IMPEM2D_BEGIN_NAMESPACE

class IMPEM2DEXPORT ProjectionOverlapFilter: public domino::SubsetFilter {

protected:

  Pointer<Image> image_;

  Pointer<ProjectionStates> states_; ----> ParticlesStatesTable !!!!
  double minimum_overlap_;

  IntPair get_position_in_image(algebra::Vector3D &position) const {
    IntPair center;
    double apix = image_->get_header().get_object_pixel_size();
    center.first = static_cast<int>(position[0]/ apix);
    center.second = static_cast<int>(position[1]/apix);
    return center;
  }

public:

  ProjectionOverlapFilter(Image *img,
                          ProjectionStates *states,
                          String name = "ProjectionOverlapFilter %1%") :
             SubsetFilter(name),
             image_(img), states_(states) {}




  void set_minimum_overlap(double percentage) {
    minimum_overlap_ = percentage;
  }

  void show(std::ostream &out = std::cout) const;

 IMP_SUBSET_FILTER(ProjectionOverlapFilter);

};
IMP_OBJECTS(ProjectionOverlapFilter,ProjectionOverlapFilters);



class IMPEM2DEXPORT DistanceFilter {

protected:
  domino::Subset my_subset_;
  domino::ParticleStatesTable ps_table_;
  double max_distance_;


public:

  DistanceFilter(const domino::Subset &subset_to_act_on,
                 const domino::ParticleStatesTable &ps_table,
                 double max_distance_) :  my_subset_(subset_to_act_on),
                                          ps_table_(ps_table),
                                          max_distance_(max_distance) {};

  void show(std::ostream &out = std::cout) const {
    std::out << "DistanceFilter" << std::endl;
  }

  IMP_SUBSET_FILTER(DistanceFilter);

};
IMP_VALUES_OR_OBJECT(DistanceFilter,DistanceFilters);

IMPEM2D_END_NAMESPACE


#endif  /* IMPEM2D_DOMINO_FILTERS_H */
