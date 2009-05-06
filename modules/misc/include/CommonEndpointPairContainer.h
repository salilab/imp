/**
 *  \file CommonEndpointPairContainer.h
 *  \brief A fake container that returns true for any pair of bonds with
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPMISC_COMMON_ENDPOINT_PAIR_CONTAINER_H
#define IMPMISC_COMMON_ENDPOINT_PAIR_CONTAINER_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/PairContainer.h>

IMPMISC_BEGIN_NAMESPACE

//! Return true for any pair of bonds sharing an endpoint
/** XXXXXX.
 */
class IMPMISCEXPORT CommonEndpointPairContainer: public PairContainer
{
public:
  CommonEndpointPairContainer();

  IMP_PAIR_CONTAINER(CommonEndpointPairContainer,internal::version_info)
};


IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_COMMON_ENDPOINT_PAIR_CONTAINER_H */
