/**
 *  \file core/BondEndpointsRefiner.h
 *  \brief Return the endpoints of a bond.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_ENDPOINTS_REFINER_H
#define IMPCORE_BOND_ENDPOINTS_REFINER_H

#include "config.h"
#include "internal/version_info.h"

#include <IMP/Refiner.h>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! Return the endpoints of a bond.
/** \deprecated Use atom::BondEndpointsRefiner
 \ingroup bond
 \see BondDecorator
 */
class IMPCOREEXPORT BondEndpointsRefiner : public Refiner
{
public:
  //! no arguments
  BondEndpointsRefiner();

  virtual ~BondEndpointsRefiner() {}

  IMP_REFINER(internal::version_info);
};

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_ENDPOINTS_REFINER_H */
