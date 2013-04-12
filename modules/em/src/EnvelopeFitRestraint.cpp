/**
 * \file EnvelopeFitRestraint.cpp
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/EnvelopeFitRestraint.h>
#include <IMP/base/warning_macros.h>

IMPEM_BEGIN_NAMESPACE

EnvelopeFitRestraint::EnvelopeFitRestraint(Particles ps,
                                           DensityMap *em_map,
                                           double density_threshold,
                                           double penetration_threshold) :
  ps_(ps),
  pca_aligner_(em_map, density_threshold),
  penetration_threshold_(penetration_threshold),
  distance_transform_(em_map, density_threshold, penetration_threshold*2.0),
  envelope_score_(&distance_transform_)
{

}

double EnvelopeFitRestraint::unprotected_evaluate(
                             IMP::DerivativeAccumulator *accum) const {
  IMP_UNUSED(accum);

  // get the XYZs
  IMP::algebra::Vector3Ds coordinates(ps_.size());
  for (unsigned int i=0; i<ps_.size(); i++) {
    coordinates[i] = core::XYZ(ps_[i]).get_coordinates();
  }

  // align
  algebra::Transformation3Ds map_transforms = pca_aligner_.align(coordinates);

  // filter and score, save best scoring only (or none if penetrating)
  float penetration_thr = -penetration_threshold_;
  bool best_found = false;
  IMP::algebra::Transformation3D best_trans;
  double best_score = -std::numeric_limits<double>::max();
  for(unsigned int j=0; j<map_transforms.size(); j++) {
      double score = envelope_score_.score(coordinates, map_transforms[j]);
      if(score > best_score) {
        best_score = score;
        best_trans = map_transforms[j];
        best_found = true;
      }
  }

  if(best_found)
    const_cast<EnvelopeFitRestraint*>(this)->transformation_ = best_trans;
  else // store identity trans
    const_cast<EnvelopeFitRestraint*>(this)->transformation_ =
      algebra::Transformation3D(algebra::Vector3D(0,0,0));

  return -best_score;
}

void EnvelopeFitRestraint::apply_transformation() {
  unprotected_evaluate(nullptr);
  for (unsigned int i=0; i<ps_.size(); i++) {
    algebra::Vector3D coord = core::XYZ(ps_[i]).get_coordinates();
    core::XYZ(ps_[i]).set_coordinates(transformation_*coord);
  }
}


IMPEM_END_NAMESPACE
