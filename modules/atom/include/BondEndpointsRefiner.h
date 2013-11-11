/**
 *  \file IMP/atom/BondEndpointsRefiner.h
 *  \brief Return the endpoints of a bond.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_BOND_ENDPOINTS_REFINER_H
#define IMPATOM_BOND_ENDPOINTS_REFINER_H

#include <IMP/atom/atom_config.h>
#include <IMP/Refiner.h>

IMPATOM_BEGIN_NAMESPACE

//! Return the endpoints of a bond.
/**
 \ingroup bond
 \see Bond
 */
class IMPATOMEXPORT BondEndpointsRefiner : public Refiner {
 public:
  //! no arguments
  BondEndpointsRefiner();

  virtual bool get_can_refine(kernel::Particle *) const IMP_OVERRIDE;
  virtual const kernel::ParticlesTemp get_refined(kernel::Particle *) const
      IMP_OVERRIDE;
#ifndef SWIG
  using Refiner::get_refined;
#endif
  virtual kernel::ModelObjectsTemp do_get_inputs(
      kernel::Model *m, const kernel::ParticleIndexes &pis) const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(BondEndpointsRefiner);
};

IMP_OBJECTS(BondEndpointsRefiner, BondEndpointsRefiners);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BOND_ENDPOINTS_REFINER_H */
