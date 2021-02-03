/**
 *  \file base_types.cpp   \brief Base types.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/nullptr.h"

namespace IMP {
#if (defined(BOOST_NO_CXX11_NULLPTR) || defined(BOOST_NO_NULLPTR)) && \
    !defined(nullptr)
const nullptr_t nullptr = nullptr_t();
#endif
}
