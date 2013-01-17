/**
 * \file EnvelopeFitRestraint.h
 * \brief score envelope fit based on map distance transform
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_ENVELOPE_FIT_RESTRAINT_H
#define IMPEM_ENVELOPE_FIT_RESTRAINT_H

#include <IMP/em/em_config.h>
#include <IMP/em/PCAAligner.h>
#include <IMP/em/MapDistanceTransform.h>
#include <IMP/em/EnvelopeScore.h>
#include <IMP/base/object_macros.h>

#include <IMP/Restraint.h>

IMPEM_BEGIN_NAMESPACE

//! A restraint for envelope-based scoring of particles in the density map
/**
   The restraint fits the particles into the density map by alignment
   of principal components of the particles with principal components
   of the map. Therefore, the particle set should comprise all
   subunits that fill the map. The aligned particles are scored using
   EnvelopeScore. This is a fast alignment and scoring that works well
   for low resolution density maps.
 */
class IMPEMEXPORT EnvelopeFitRestraint : public IMP::Restraint {
public:
  //! Constructor
  /**
    \param[in] particles The particles participating in the score
    \param[in] em_map The density map used in the fitting score
    \param[in] density_threshold Density map threshold
    \param[in] penetration_threshold A distance for which penetration
    of particles out of the map envelope (as defined by density_threshold)
    is allowed. Recommended value is approximately map resolution
  */
  EnvelopeFitRestraint(Particles particles,
                       DensityMap *em_map,
                       double density_threshold,
                       double penetration_threshold);

  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const;

  // compute and return best scoring transofrmation
  algebra::Transformation3D get_transformation() const {
    unprotected_evaluate(nullptr);
    return transformation_;
  }

  // compute best scoring transformation and apply it on the particles
  void apply_transformation();

  IMP::ModelObjectsTemp do_get_inputs() const { return ps_; }
  IMP_OBJECT_METHODS(EnvelopeFitRestraint);

 private:
  Particles ps_;
  PCAAligner pca_aligner_;
  double penetration_threshold_;
  MapDistanceTransform distance_transform_;
  EnvelopeScore envelope_score_;

  // best scoring trans from last evaluate
  IMP::algebra::Transformation3D transformation_;
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_ENVELOPE_FIT_RESTRAINT_H */
