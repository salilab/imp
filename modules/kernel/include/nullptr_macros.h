/**
 *  \file IMP/nullptr_macros.h
 *  \brief Provide a nullptr keyword analog.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_NULLPTR_MACROS_H
#define IMPKERNEL_NULLPTR_MACROS_H

#include <IMP/kernel_config.h>

#if(defined(BOOST_NO_CXX11_NULLPTR) || defined(BOOST_NO_NULLPTR)) && \
    !defined(nullptr)
#define IMP_NULLPTR IMP::nullptr
#define IMP_NULLPTR_T IMP::nullptr_t
#else
#define IMP_NULLPTR nullptr
#define IMP_NULLPTR_T std::nullptr_t
#endif  // IMP_COMPILER_HAS_NULLPTR

#endif /* IMPKERNEL_NULLPTR_MACROS_H */
