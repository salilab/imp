/**
 *  \file base_types.cpp   \brief Base types.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/base/nullptr.h"

namespace IMP {
#if defined(BOOST_NO_CXX11_NULLPTR)
  const nullptr_t nullptr = nullptr_t();
#endif
}
