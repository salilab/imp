/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_HASH_H
#define RMF_INTERNAL_HASH_H

#include "RMF/config.h"

RMF_PUSH_WARNINGS
RMF_CLANG_PRAGMA(diagnostic ignored "-Wmismatched-tags")
#include <boost/functional/hash.hpp>       // IWYU pragma: export
#include <boost/functional/hash/hash.hpp>  // IWYU pragma: export
RMF_POP_WARNINGS

#endif /* RMF_INTERNAL_HASH_H */
