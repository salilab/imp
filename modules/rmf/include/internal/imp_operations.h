/**
 *  \file IMP/rmf/operations.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INTERNAL_IMP_OPERATIONS_H
#define IMPRMF_INTERNAL_IMP_OPERATIONS_H

#include "../rmf_config.h"
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/types.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

/** Get the requested key, adding it to the file if necessary.
 */
template <class TypeT>
inline RMF::Key<TypeT, 1> get_or_add_key(RMF::FileHandle f,
                                      RMF::Category category_id,
                                      std::string name,
                                      bool per_frame=false) {
  if (f.get_has_key<TypeT, 1>(category_id, name)) {
    return f.get_key<TypeT, 1>(category_id, name);
  } else {
    return f.add_key<TypeT, 1>(category_id, name, per_frame);
  }
}

template <class TypeT>
inline RMF::Key<TypeT, 1> get_or_add_key(RMF::FileConstHandle f,
                                      RMF::Category category_id,
                                      std::string name,
                                      bool =false) {
  return f.get_key<TypeT, 1>(category_id, name);
}


template <int Arity>
inline RMF::CategoryD<Arity> get_or_add_category(RMF::FileHandle f,
                                                std::string name) {
  if (f.get_has_category<Arity>(name)) {
    return f.get_category<Arity>(name);
  } else {
    return f.add_category<Arity>(name);
  }
}


template <int Arity>
inline RMF::CategoryD<Arity> get_or_add_category(RMF::FileConstHandle f,
                                                 std::string name) {
  return f.get_category<Arity>(name);
}


IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_INTERNAL_IMP_OPERATIONS_H */
