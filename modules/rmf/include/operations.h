/**
 *  \file IMP/rmf/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_OPERATIONS_H
#define IMPRMF_OPERATIONS_H

#include "rmf_config.h"
#include "NodeHandle.h"
#include "RootHandle.h"
#include "hdf5_types.h"

IMPRMF_BEGIN_NAMESPACE

/** Get the requested key, adding it to the file if necessary.
 */
template <class TypeT>
inline Key<TypeT> get_or_add_key(RootHandle f, Category category_id,
                                 std::string name,
                                 bool per_frame=false) {
  if (f.get_has_key<TypeT>(category_id, name)) {
    return f.get_key<TypeT>(category_id, name);
  } else {
    return f.add_key<TypeT>(category_id, name, per_frame);
  }
}


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_OPERATIONS_H */
