/**
 *  \file RMF/keys.h
 *  \brief Declarations of the various key types.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_KEYS_H
#define RMF_KEYS_H

#include "RMF/config.h"
#include "ID.h"
#include "traits.h"
#include "infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {

#define RMF_DECLARE_KEY(TT, UCName)          \
  /** Identify a value stored for a node. */ \
  typedef ID<UCName##Tag> UCName##Key;       \
  /** A list of identifiers. */              \
  typedef std::vector<UCName##Key> UCName##Keys;

/** \name Key types
    RMF files support storing a variety of different types of data. These
    include
    - IntKey: store an arbitrary integer as a 64 bit integer
    - FloatKey: store an arbitrary floating point number as a `float`
    - StringKey: store an arbitrary length string
    - IndexKey: store non-negative indexes as 64 bit integers
    - NodeIDKey: store the ID of a node in the hierarchy
    - NodeIDsKey: store a list of NodeIDs
    - DataSetKey: store a reference to an HDF5 data set via the path to the
    data set
    - IntsKey: store a list of arbitrary integers
    @{
 */
RMF_FOREACH_TYPE(RMF_DECLARE_KEY);
/** @} */

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_KEYS_H */
