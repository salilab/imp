/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPLIBRMF_INTERNAL_UTILITY_H
#define IMPLIBRMF_INTERNAL_UTILITY_H

#include "../RMF_config.h"

namespace RMF {
  namespace internal {
    template <class VA, class VB>
    void copy_vector(const VA &va,
                     VB &vb) {
      vb.insert(vb.end(), va.begin(), va.end());
    }
  }
}


#endif  /* IMPLIBRMF_INTERNAL_UTILITY_H */
