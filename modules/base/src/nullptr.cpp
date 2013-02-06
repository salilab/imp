/**
 *  \file base_types.cpp   \brief Base types.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/nullptr.h"

namespace IMP {

#if !IMP_COMPILER_HAS_NULLPTR
const nullptr_t nullptr=nullptr_t();
#endif

}
