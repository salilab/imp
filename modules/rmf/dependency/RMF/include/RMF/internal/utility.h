/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_UTILITY_H
#define RMF_INTERNAL_UTILITY_H

#include <RMF/config.h>

RMF_ENABLE_WARNINGS

namespace RMF {
class AliasFactory;
class NodeHandle;
class NodeConstHandle;

namespace internal {
template <class VA, class VB>
void copy_vector(const VA &va,
                 VB       &vb) {
  vb.insert(vb.end(), va.begin(), va.end());
}


RMFEXPORT NodeHandle add_child_alias(AliasFactory    af,
                                     NodeHandle      parent,
                                     NodeConstHandle child);
}
}

RMF_DISABLE_WARNINGS

#endif  /* RMF_INTERNAL_UTILITY_H */
