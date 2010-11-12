/**
 *  \file density_utilities.h
 *  \brief density map manipulations
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_DENSITY_UTILITIES_H
#define IMPEM_DENSITY_UTILITIES_H


#include "em_config.h"
#include <IMP/base_types.h>
#include "DensityMap.h"

IMPEM_BEGIN_NAMESPACE

//! Return a masked density , and
//! normalize the output map within the masked region
/**
\param[in] dmap the density map to mask
\param[in] mask the mask
\param[out] the masked and normalized map
 */
IMPEMEXPORT DensityMap* mask_and_norm(em::DensityMap *dmap,
                                         em::DensityMap *mask);
IMPEM_END_NAMESPACE
#endif  /* IMPEM_DENSITY_UTILITIES_H */
