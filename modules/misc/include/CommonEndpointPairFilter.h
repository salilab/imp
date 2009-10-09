/**
 *  \file CommonEndpointPairFilter.h
 *  \brief A fake filter that returns true for any pair of bonds with
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H
#define IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H

#include "config.h"

#include <IMP/PairFilter.h>

IMPMISC_BEGIN_NAMESPACE

//! Return true for any pair of bonds sharing an endpoint
/** XXXXXX.
 */
class IMPMISCEXPORT CommonEndpointPairFilter: public PairFilter
{
public:
  CommonEndpointPairFilter();

  IMP_PAIR_FILTER(CommonEndpointPairFilter,get_module_version_info())
};


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H */
