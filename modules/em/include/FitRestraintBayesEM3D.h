/**
 *  \file IMP/em/FitRestraintBayesEM3D.h
 *  \brief Calculate the Bayesian score and derivative 
 *   based on fit to an EM density map. This restraint differs from em::FitRestraint 
 *   because it does not use a cross_correlation_coefficient as score but a Bayesian 
 *   similarity measure. 
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_FIT_RESTRAINT_BAYES_EM3D_H
#define IMPEM_FIT_RESTRAINT_BAYES_EM3D_H

#include <IMP/em/em_config.h>
#include "DensityMap.h"
#include "BayesEM3D.h"
#include "SampledDensityMap.h"

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZR.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/Refiner.h>
#include <boost/unordered_map.hpp>

IMPEM_BEGIN_NAMESPACE

class IMPEMEXPORT FitRestraintBayesEM3D : public Restraint {
  //! Calculate score based on fit to EM map
  /** The score is a function of the difference between
      the normalize EM map and the map simulated from the particles. 
      This restraint is numerically stable and 
      incorporate knowledge about the uncertainty in the EM map.
      Note: This is still in development.
      \ingroup exp_restraint
  */

public:
  //! Constructor
  FitRestraintBayesEM3D(
    ParticlesTemp ps,
    DensityMap *em_map,
    FloatKey weight_key = atom::Mass::get_mass_key(),
    bool use_rigid_bodies = true, 
    double sigma = .1);
  
  virtual double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
  const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(FitRestraintBayesEM3D);

private:

  IMP::PointerMember<DensityMap> target_dens_map_;
  double resolution_;
  double voxel_size_;
  double sigma_;

  // reference to the IMP environment
  core::XYZs xyz_;

  // score and derivatives
  IMP::PointerMember<BayesEM3D> bayesem3d_;
  double score_;
  algebra::Vector3Ds dv_;

  // Particles
  Particles ps_;
  FloatKey weight_key_;

};

IMPEM_END_NAMESPACE

#endif /* IMPEM_FIT_RESTRAINT_BAYES_EM3D_H */
