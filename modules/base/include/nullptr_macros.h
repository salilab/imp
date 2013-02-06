/**
 *  \file IMP/base/nullptr_macros.h
 *  \brief Provide a nullptr keyword analog.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_NULLPTR_MACROS_H
#define IMPBASE_NULLPTR_MACROS_H

#include <IMP/base/base_config.h>

#if IMP_COMPILER_HAS_NULLPTR
#define IMP_NULLPTR nullptr
#define IMP_NULLPTR_T std::nullptr_t
#else
#define IMP_NULLPTR IMP::nullptr
#define IMP_NULLPTR_T IMP::nullptr_t
#endif // IMP_COMPILER_HAS_NULLPTR

#endif  /* IMPBASE_NULLPTR_MACROS_H */
