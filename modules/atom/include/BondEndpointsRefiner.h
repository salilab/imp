/**
 *  \file atom/BondEndpointsRefiner.h
 *  \brief Return the endpoints of a bond.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPATOM_BOND_ENDPOINTS_REFINER_H
#define IMPATOM_BOND_ENDPOINTS_REFINER_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Refiner.h>

IMPATOM_BEGIN_NAMESPACE

//! Return the endpoints of a bond.
/**
 \ingroup bond
 \see BondDecorator
 */
class IMPATOMEXPORT BondEndpointsRefiner : public Refiner
{
public:
  //! no arguments
  BondEndpointsRefiner();

  virtual ~BondEndpointsRefiner() {}

  IMP_REFINER(internal::version_info);
};

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_BOND_ENDPOINTS_REFINER_H */
