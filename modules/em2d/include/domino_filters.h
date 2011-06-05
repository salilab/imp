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

  /*! Retrun true in the projection corresponding to the assignment
    overlaps enough with an image (there is density in both of them)
  */
  bool get_is_ok(const domino::Assignment &assignment) const {
    IMP_USAGE_CHECK(assignment.size() == 1, "The filter is applied only to one"
                                          "particle");
    unsigned int index = assignment[0];
    Pointer<Image> projection = states_->get_projection(index);

    // Get the pixel of the image that corresponds to the position
    // specified by the assignment
    algebra::Vector3D v = states_->get_position(index);
    IntPair center = get_position_in_image( v );
    double overlap = get_overlap_percentage(image_->get_data(),
                                            projection->get_data(),
                                            center);
    if(overlap >= minimum_overlap_) return true;
    return false;
  };

  IMP_OBJECT(ProjectionOverlapFilter);

};
IMP_OBJECTS(ProjectionOverlapFilter,ProjectionOverlapFilters);

IMPEM2D_END_NAMESPACE


#endif  /* IMPEM2D_DOMINO_FILTERS_H */
