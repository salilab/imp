/**
 *  \file IMP/utility.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_INTERNAL_UTILITY_H
#define IMPRMF_INTERNAL_UTILITY_H
#include <IMP/exception.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE
IMPRMF_END_INTERNAL_NAMESPACE

#define IMP_RMF_USAGE_CHECK(check, message)\
  IMP_USAGE_CHECK(check, message)

#define IMP_RMF_INTERNAL_CHECK(check, message)\
  IMP_INTERNAL_CHECK(check, message)

#define IMP_RMF_IF_CHECK\
  IMP_IF_CHECK(USAGE)

#define IMP_RMF_THROW(m,e)\
  IMP_THROW(m,e)

/** Call a function and throw an exception if the return values is bad */
#if IMP_BUILD < IMP_FAST
#define IMP_HDF5_CALL(v) IMP_RMF_USAGE_CHECK((v)>=0, "Error calling "<< (#v))
#else
#define IMP_HDF5_CALL(v) if((v)<0) {                    \
    IMP_RMF_THROW("Error calling "<< (#v), ValueException); \
  }
#endif


#endif /* IMPRMF_INTERNAL_UTILITY_H */
