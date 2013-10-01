/**
 *  \file IMP/core/ChildrenRefiner.h
 *  \brief Return the hierarchy children of a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_CHILDREN_REFINER_H
#define IMPCORE_CHILDREN_REFINER_H

#include <IMP/core/core_config.h>
#include "Hierarchy.h"

#include <IMP/Refiner.h>
#include <IMP/refiner_macros.h>

IMPCORE_BEGIN_NAMESPACE

class HierarchyTraits;

//! Return the hierarchy children of a particle.
/** \ingroup hierarchy
    A simple example using is
    \include cover_particles.py
    \see Hierarchy
    \see Hierarchy
*/
class IMPCOREEXPORT ChildrenRefiner : public Refiner {

  HierarchyTraits traits_;

 public:
  //! Create a refiner for a particular type of hierarchy
  ChildrenRefiner(HierarchyTraits tr);

  virtual bool get_can_refine(kernel::Particle *) const IMP_OVERRIDE;
  virtual const kernel::ParticlesTemp get_refined(kernel::Particle *) const
      IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ChildrenRefiner);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CHILDREN_REFINER_H */
