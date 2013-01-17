/**
 *  \file IMP/em/PCAFitRestraint.h
 *  \brief Calculate match between density map PCA and particles PCA.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_PCA_FIT_RESTRAINT_H
#define IMPEM_PCA_FIT_RESTRAINT_H

#include <IMP/em/em_config.h>
#include "DensityMap.h"
#include "CoarseCC.h"
#include "SampledDensityMap.h"

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/XYZR.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>
#include <IMP/Refiner.h>
#include <IMP/algebra/eigen_analysis.h>

IMPEM_BEGIN_NAMESPACE

//! Calculate score based on fit to EM map
/** \ingroup exp_restraint

 */
class IMPEMEXPORT PCAFitRestraint : public Restraint
{
public:
  //! Constructor
  /**
    \param[in] ps The particles participating in the fitting score
    \param[in] em_map  The density map used in the fitting score
    \param[in] threahold consider all voxels above this value for
               PCA calculation
    \param[in] weight_key the name of the weight attribute of the particles
   */
  PCAFitRestraint(ParticlesTemp ps,
       DensityMap *em_map, float threahold,
       float max_pca_size_diff,float max_angle_diff,float max_centroid_diff,
       FloatKey weight_key= atom::Mass::get_mass_key());

  IMP_RESTRAINT(PCAFitRestraint);

#ifndef SWIG
  IMP_LIST(private, Particle, particle, Particle*, Particles);
#endif
private:
  //! Store particles
  void store_particles(ParticlesTemp ps);

  IMP::OwnerPointer<DensityMap> target_dens_map_;
  float threshold_;
  algebra::BoundingBoxD<3> target_bounding_box_;
  // reference to the IMP environment
  core::XYZs xyz_;
  FloatKey weight_key_;
  Particles all_ps_;
  KernelType kt_;
  algebra::PrincipalComponentAnalysis dens_pca_;
  float max_angle_diff_,max_pca_size_diff_;
  float max_centroid_diff_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_PCA_FIT_RESTRAINT_H */
