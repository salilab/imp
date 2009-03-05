/**
 *  \file EMFitRestraint.h
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPEM_EM_FIT_RESTRAINT_H
#define IMPEM_EM_FIT_RESTRAINT_H

#include "config.h"
#include "internal/version_info.h"
#include "DensityMap.h"
#include "CoarseCC.h"
#include "SampledDensityMap.h"
#include "IMPParticlesAccessPoint.h"

#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/VersionInfo.h>

IMPEM_BEGIN_NAMESPACE

//! Calculate score based on fit to EM map
/** \ingroup exp_restraint

 */
class IMPEMEXPORT EMFitRestraint : public Restraint
{
public:
  //! Constructor
  /**
    \param[in] ps The particles participating in the fitting score
    \param[in] em_map  The density map used in the fitting score
    \param[in] radius_key the name of the radius attribute of the particles
    \param[in] weight_key the name of the weight attribute of the particles
    \param[in] scale
   */
  EMFitRestraint(const Particles &ps,
                 DensityMap &em_map,
                 FloatKey radius_key,
                 FloatKey weight_key,
                 float scale);

  //! \return the predicted density map of the model
  SampledDensityMap *get_model_dens_map() {
    return model_dens_map_;
  }

  IMP_RESTRAINT(EMFitRestraint, internal::version_info)

  ParticlesList get_interacting_particles() const
  {
    return ParticlesList(1, Particles(particles_begin(), particles_end()));
  }

  IMP_LIST(private, Particle, particle, Particle*)
private:
  DensityMap *target_dens_map_;
  SampledDensityMap *model_dens_map_;
  // reference to the IMP environment
  float scalefac_;
  int num_particles_; // can it be removed ?
  IMPParticlesAccessPoint access_p_;
  // derivatives
  // This must be float rather than Float to preserve compatibility with EMBed
  std::vector<float> dx_, dy_ , dz_;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_EM_FIT_RESTRAINT_H */
