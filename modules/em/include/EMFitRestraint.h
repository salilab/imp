/**
 *  \file EMFitRestraint.cpp
 *  \brief Calculate score based on fit to EM map.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPEM_EM_FIT_RESTRAINT_H
#define __IMPEM_EM_FIT_RESTRAINT_H

#include "em_exports.h"
#include "em_version_info.h"
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

  EMFitRestraint(Model& model,
                 std::vector<int>& particle_indexes, //the selection
                 DensityMap &em_map,
                 std::string radius_str,
                 std::string weight_str,
                 float scale);

  virtual ~EMFitRestraint();

  //! \return the predicted density map of the model
  SampledDensityMap *get_model_dens_map() {
    return model_dens_map_;
  }

  IMP_RESTRAINT(internal::em_version_info)

private:

  DensityMap *target_dens_map_;
  SampledDensityMap *model_dens_map_;
  // reference to the IMP environment
  float scalefac_;
  int num_particles_;
  IMPParticlesAccessPoint access_p_;
  // derivatives
  std::vector<Float> dx_, dy_ , dz_;
};

IMPEM_END_NAMESPACE

#endif  /* __IMPEM_EM_FIT_RESTRAINT_H */
