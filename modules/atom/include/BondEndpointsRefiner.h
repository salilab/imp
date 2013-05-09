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
#include <IMP/refiner_macros.h>
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

  IMP_REFINER(BondEndpointsRefiner);
};

IMP_OBJECTS(BondEndpointsRefiner, BondEndpointsRefiners);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_BOND_ENDPOINTS_REFINER_H */
