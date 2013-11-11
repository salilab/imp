/**
 *  \file IMP/core/LeavesRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_LEAVES_REFINER_H
#define IMPCORE_LEAVES_REFINER_H

#include <IMP/core/core_config.h>
#include "Hierarchy.h"

#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

class HierarchyTraits;

//! Return the hierarchy leaves under a particle.
/** \ingroup hierarchy
    \see Hierarchy
    \see Hierarchy
*/
class IMPCOREEXPORT LeavesRefiner : public Refiner {
  HierarchyTraits traits_;

 public:
  //! Create a refiner for a particular type of hierarchy
  LeavesRefiner(HierarchyTraits tr);

  virtual bool get_can_refine(kernel::Particle *) const IMP_OVERRIDE;
  virtual const kernel::ParticlesTemp get_refined(kernel::Particle *) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  IMP_OBJECT_METHODS(LeavesRefiner);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_LEAVES_REFINER_H */
