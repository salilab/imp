/**
 *  \file IMP/hdf5/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPHDF5_OPERATIONS_H
#define IMPHDF5_OPERATIONS_H

#include "hdf5_config.h"
#include "NodeHandle.h"

IMPHDF5_BEGIN_NAMESPACE

/** Get the requested key, adding it to the file if necessary.
 */
template <class TypeT>
inline Key<TypeT> get_or_add_key(RootHandle f, KeyCategory category_id,
                                 std::string name,
                                 bool per_frame=false) {
  if (f.get_has_key<TypeT>(category_id, name)) {
    return f.get_key<TypeT>(category_id, name);
  } else {
    return f.add_key<TypeT>(category_id, name, per_frame);
  }
}


IMPHDF5_END_NAMESPACE

#endif /* IMPHDF5_OPERATIONS_H */
