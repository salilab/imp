/**
 *  \file RMF/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__INTERNAL_RMF_OPERATIONS_H
#define RMF__INTERNAL_RMF_OPERATIONS_H

#include <RMF/config.h>
#include "../NodeHandle.h"
#include "../FileHandle.h"
#include "../types.h"

RMF__BEGIN_INTERNAL_NAMESPACE

/** Get the requested key, adding it to the file if necessary.
 */
template <class TypeT>
inline RMF::Key<TypeT> get_or_add_key(RMF::FileHandle f,
                                        RMF::Category category_id,
                                        std::string name,
                                        bool per_frame=false) {
  if (f.get_has_key<TypeT>(category_id, name)) {
    return f.get_key<TypeT>(category_id, name);
  } else {
    return f.add_key<TypeT>(category_id, name, per_frame);
  }
}


RMF__END_INTERNAL_NAMESPACE

#endif /* RMF__INTERNAL_RMF_OPERATIONS_H */
