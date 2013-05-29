/**
 *  \file IMP/statistics/embedding_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_EMBEDDING_MACROS_H
#define IMPSTATISTICS_EMBEDDING_MACROS_H

#include "Embedding.h"
#include <IMP/base/object_macros.h>

//! Define things needed for a Embedding
/** In addition to what is done by IMP_OBJECT() it declares
    - IMP::statistics::Embedding::get_point()
    - IMP::statistics::Embedding::get_number_of_items()
    and defines
    - IMP::statistics::Embedding::get_points()
*/
#define IMP_EMBEDDING(Name)                          \
  algebra::VectorKD get_point(unsigned int i) const; \
  algebra::VectorKDs get_points() const {            \
    algebra::VectorKDs ret(get_number_of_items());   \
    for (unsigned int i = 0; i < ret.size(); ++i) {  \
      ret[i] = Name::get_point(i);                   \
    }                                                \
    return ret;                                      \
  }                                                  \
  unsigned int get_number_of_items() const;          \
  IMP_OBJECT(Name)

#endif /* IMPSTATISTICS_EMBEDDING_MACROS_H */
