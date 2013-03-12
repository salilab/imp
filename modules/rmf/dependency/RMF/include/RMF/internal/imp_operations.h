/**
 *  \file RMF/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_RMF_OPERATIONS_H
#define RMF_INTERNAL_RMF_OPERATIONS_H

#include <RMF/config.h>
#include "../NodeHandle.h"
#include "../FileHandle.h"
#include "../types.h"

RMF_ENABLE_WARNINGS

namespace RMF {
  namespace internal {
/** Get the requested key, adding it to the file if necessary.
 */
template <class TypeT>
inline RMF::Key<TypeT> get_or_add_key(RMF::FileHandle f,
                                      RMF::Category   category_id,
                                      std::string     name,
                                      bool            per_frame = false) {
  if (f.get_has_key<TypeT>(category_id, name)) {
    return f.get_key<TypeT>(category_id, name);
  } else {
    return f.add_key<TypeT>(category_id, name, per_frame);
  }
}


  }
}

RMF_DISABLE_WARNINGS


#endif /* RMF_INTERNAL_RMF_OPERATIONS_H */
