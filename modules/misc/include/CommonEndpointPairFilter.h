/**
 *  \file IMP/misc/CommonEndpointPairFilter.h
 *  \brief A fake filter that returns true for any pair of bonds with
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H
#define IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H

#include "misc_config.h"

#include <IMP/PairPredicate.h>
#include <IMP/pair_macros.h>
IMPMISC_BEGIN_NAMESPACE

//! Return true for any pair of bonds sharing an endpoint
/** XXXXXX.
 */
class IMPMISCEXPORT CommonEndpointPairFilter: public PairPredicate
{
public:
  CommonEndpointPairFilter();

  IMP_PAIR_PREDICATE(CommonEndpointPairFilter);
};


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_COMMON_ENDPOINT_PAIR_FILTER_H */
