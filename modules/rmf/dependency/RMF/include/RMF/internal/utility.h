/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef RMF__INTERNAL_UTILITY_H
#define RMF__INTERNAL_UTILITY_H

#include <RMF/config.h>

namespace RMF {
  class AliasFactory;
  class NodeHandle;
  class NodeConstHandle;

  namespace internal {
    template <class VA, class VB>
    void copy_vector(const VA &va,
                     VB &vb) {
      vb.insert(vb.end(), va.begin(), va.end());
    }


    RMFEXPORT NodeHandle add_child_alias(AliasFactory af,
                                         NodeHandle parent,
                                         NodeConstHandle child);
  }
}


#endif  /* RMF__INTERNAL_UTILITY_H */
